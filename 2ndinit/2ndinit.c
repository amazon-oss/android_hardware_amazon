/*
 * Copyright (c) 2016 rbox
 * Copyright (c) 2024 R0rt1z2
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dirent.h>
#include <elf.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/netlink.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/ptrace.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#include "archive.h"
#include "archive_entry.h"
#include "module.h"
#include "sepol/policydb/services.h"

#define NETLINK_BUFFER_SIZE 512
#define UEVENT_BUFFER_SIZE 2048

#define KEY_DEVICE "/dev/input/event0"
#define USB_DEVICE "/dev/block/sda"

#define MNT_PATH "/mnt/"
#define SYSTEM_PATH "/system/etc/"

#define DAEMONSU "/system/xbin/daemonsu"

#define NORMAL_RAMDISK MNT_PATH "ramdisk.cpio.lzma"
#define RECOVERY_RAMDISK MNT_PATH "ramdisk-recovery.cpio.lzma"

#define SYSTEM_NORMAL_RAMDISK SYSTEM_PATH "ramdisk.cpio.lzma"
#define SYSTEM_RECOVERY_RAMDISK SYSTEM_PATH "ramdisk-recovery.cpio.lzma"

#define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)

typedef struct {
    int volume_up_detected;
    int volume_down_detected;
} key_state_t;

int boot_mode = 0;
char *ramdisk_path = NULL;

static void print(const char *format, ...) {
    va_list args;
    int fd;
    char msg[256];

    fd = open("/dev/kmsg", O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open /dev/kmsg: %s\n", strerror(errno));
        return;
    }

    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);

    if (write(fd, msg, strlen(msg)) < 0) {
        fprintf(stderr, "Failed to write to /dev/kmsg: %s\n", strerror(errno));
    }

    close(fd);
}

static int get_boot_mode() {
    int fd = -1;
    char buffer[16];

    fd = open("/sys/devices/virtual/BOOT/BOOT/boot/boot_mode", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open boot mode file");
        exit(EXIT_FAILURE);
    }

    int bytes = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);

    if (bytes < 0) {
        print("Failed to read from boot mode file");
        return 0; // Assume normal boot mode
    }

    buffer[bytes] = '\0';
    return atoi(buffer);
}

static int init_netlink() {
    struct sockaddr_nl addr;
    int sock, ret;

    sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (sock < 0) {
        print("Unable to create socket: %s\n", strerror(errno));
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 1;

    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        print("Unable to bind socket: %s\n", strerror(errno));
        close(sock);
        return -1;
    }

    return sock;
}

static void check_secondary_storage() {
    struct stat sStat;
    char target_ramdisk_path[256];
    int sock = init_netlink();
    if (sock < 0) {
        print("Failed to create netlink socket\n");
        return;
    }

    print("Listening for device uevents...\n");
    while (1) {
        char buf[UEVENT_BUFFER_SIZE] = {0};
        ssize_t len = recv(sock, buf, sizeof(buf) - 1, 0);
        if (len < 0) {
            print("Error reading from netlink socket: %s\n", strerror(errno));
            continue;
        }

        buf[len] = '\0';
        char *ptr = buf;
        while (ptr < buf + len) {
            if (strstr(ptr, "add@/devices") && strstr(ptr, "/block/sda")) {
                print("Device %s is ready!\n", USB_DEVICE);
                sleep(1); // Better safe than sorry ;)

                int mountResult = mount(USB_DEVICE, MNT_PATH, "vfat", 0, NULL);
                if (mountResult == 0) {
                    print("Mounted %s successfully\n", USB_DEVICE);
                    strcpy(target_ramdisk_path,
                           boot_mode == 2 ? RECOVERY_RAMDISK : NORMAL_RAMDISK);
                    if (stat(target_ramdisk_path, &sStat) == 0) {
                        print("Found ramdisk on %s!\n", target_ramdisk_path);
                        ramdisk_path = target_ramdisk_path;
                        return;
                    } else {
                        print("No ramdisk found on %s: %s\n",
                              target_ramdisk_path, strerror(errno));
                    }
                } else {
                    print("Failed to mount %s: %s\n", USB_DEVICE,
                          strerror(errno));
                }
                break;
            }
            ptr += strlen(ptr) + 1;
        }
    }

    close(sock);
}

static void *key_listen(void *arg) {
    int fd;
    struct input_event ev;
    key_state_t *key_state = (key_state_t *)arg;

    fd = open(KEY_DEVICE, O_RDONLY);
    if (fd < 0) {
        print("Error opening device %s: %s\n", KEY_DEVICE, strerror(errno));
        return NULL;
    } else {
        print("Opened device %s successfully.\n", KEY_DEVICE);
    }

    while (read(fd, &ev, sizeof(ev)) > 0) {
        print("Event: type %d, code %d, value %d\n", ev.type, ev.code,
              ev.value);

        if (ev.type == EV_KEY && ev.value == 1) {
            print("Key press detected: code %d\n", ev.code);

            if (ev.code == KEY_VOLUMEUP) {
                print("Volume Up key detected!\n");
                key_state->volume_up_detected = 1;
                break;
            } else if (ev.code == KEY_VOLUMEDOWN) {
                print("Volume Down key detected!\n");
                key_state->volume_down_detected = 1;
                break;
            }
        }
    }

    if (errno) {
        print("Error reading from device %s: %s\n", KEY_DEVICE,
              strerror(errno));
    }

    close(fd);
    return NULL;
}

static void set_permissive(policydb_t *pdb) {
    struct hashtab_node *cur;
    size_t i;
    for (i = 0; i < pdb->p_types.table->size; i++) {
        for (cur = pdb->p_types.table->htable[i]; cur != NULL;
             cur = cur->next) {
            type_datum_t *type = cur->datum;
            if (type != NULL) {
                ebitmap_set_bit(&pdb->permissive_map, type->s.value, 1);
            }
        }
    }
}

static void selinux_permissive(char *src, char *dst, int load) {
    policydb_t policydb;
    struct policy_file pf;

    print("Initializing policy database...\n");
    policydb_init(&policydb);
    sepol_set_policydb(&policydb);

    print("Reading current policy...\n");
    pf.fp = fopen(src, "r");
    if (!pf.fp) {
        print("Unable to open %s!\n", src);
        return;
    }

    pf.type = PF_USE_STDIO;
    if (policydb_read(&policydb, &pf, 0)) {
        print("Unable to read policydb!\n");
        fclose(pf.fp);
        return;
    }

    fclose(pf.fp);

    print("Making all domains permissive...\n");
    set_permissive(&policydb);

    print("Writing new policy...\n");
    pf.fp = fopen(dst, "w+");
    if (!pf.fp) {
        print("Unable to open %s!\n", dst);
        policydb_destroy(&policydb);
        return;
    }

    if (policydb_write(&policydb, &pf)) {
        print("Unable to write policydb\n!");
        fclose(pf.fp);
        policydb_destroy(&policydb);
        return;
    }

    if (load) {
        print("Loading new policy...\n");
        int size = ftell(pf.fp);
        fseek(pf.fp, SEEK_SET, 0);
        void *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fileno(pf.fp), 0);
        if (map == MAP_FAILED) {
            print("Unable to map memory for policy!\n");
            fclose(pf.fp);
            policydb_destroy(&policydb);
            return;
        }
        int load = open("/sys/fs/selinux/load", O_WRONLY);
        if (load < 0) {
            print("Unable to open /sys/fs/selinux/load!\n");
            munmap(map, size);
            fclose(pf.fp);
            policydb_destroy(&policydb);
            return;
        }
        write(load, map, size);
        close(load);
        munmap(map, size);
    }
}

static void unmount(const char *path) {
    if (umount(path) < 0) {
        if (umount2(path, MNT_DETACH) < 0) {
            print("Failed to unmount %s: %s!\n", path, strerror(errno));
        }
    }
}

static void extract_ramdisk(char *ramdisk_path) {
    // Delete everything in the current root
    print("Cleaning up current root...\n");
    DIR *root = opendir("/");
    if (!root) {
        print("Unable to open root directory!\n");
        return;
    }

    struct dirent *dp;
    while ((dp = readdir(root)) != NULL) {
        if (unlink(dp->d_name) < 0) {
            print("Unable to unlink %s!\n", dp->d_name);
        }
    }
    closedir(root);

    // Open the ramdisk
    print("Extracting ramdisk from %s...\n", ramdisk_path);
    struct archive *a = archive_read_new();
    archive_read_support_filter_lzma(a);
    archive_read_support_format_cpio(a);
    if (archive_read_open_filename(a, ramdisk_path, 4096) == ARCHIVE_OK) {
        // Set up the writer
        struct archive *ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_UNLINK |
                                                ARCHIVE_EXTRACT_PERM);
        chdir("/");

        while (1) {
            struct archive_entry *entry;

            // Read the next file
            if (archive_read_next_header(a, &entry) == ARCHIVE_EOF)
                break;

            // Create the file
            archive_write_header(ext, entry);

            // If the file has data to write...
            if (!archive_entry_size_is_set(entry) ||
                archive_entry_size(entry) > 0) {
                const void *block;
                size_t size;
                int64_t offset;

                // Write the blocks until EOF
                while (1) {
                    if (archive_read_data_block(a, &block, &size, &offset) ==
                        ARCHIVE_EOF)
                        break;
                    archive_write_data_block(ext, block, size, offset);
                }
            }
        }

        archive_write_free(ext);
    }

    archive_read_free(a);
}

static void read_init_map(char *wanted_dev, unsigned long *base) {
    char line[128];

    FILE *fp = fopen("/proc/1/maps", "r");
    if (!fp) {
        print("Failed to open /proc/1/maps\n");
        return;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, wanted_dev)) {
            sscanf(line, "%lx", base);
            break;
        }
    }
    fclose(fp);
}

static unsigned long find_execve(unsigned long image_base) {
#ifdef __aarch64__
    /*===================================
     * execve:
     *
     * d2801ba8  mov   x8, #0xdd
     * d4000001  svc   #0x0
     * b140041f  cmn   x0, #0x1, lsl #12
     * da809400  cneg  x0, x0, hi
     *=================================*/
    const int execve_code[] = {0xd2801ba8, 0xd4000001, 0xb140041f, 0xda809400};
#else
    /*====================================
     * execve:
     *
     * e1a0c007        mov     ip, r7
     * e3a0700b        mov     r7, #11
     *==================================*/
    const int execve_code[] = {0xe1a0c007, 0xe3a0700b};
#endif
    unsigned long i;

    for (i = 0;; i += sizeof(*execve_code)) {
        unsigned long buffer[2];

        // Read the next 8 or 16 bytes
        buffer[0] = ptrace(PTRACE_PEEKTEXT, 1, image_base + i, NULL);
        buffer[1] =
            ptrace(PTRACE_PEEKTEXT, 1, image_base + i + sizeof(*buffer), NULL);

        // compare them to the execve instructions
        if (memcmp(buffer, execve_code, sizeof(buffer)) == 0) {
            // Found the address of execve
            print("Found execve at address: 0x%lx\n", image_base + i);
            return image_base + i;
        }
    }

    // execve not found
    print("Unable to find excve!\n");
    return 0;
}

static void replace_init(void) {
    // Attach to existing init and wait for an interrupt
    if (ptrace(PTRACE_ATTACH, 1, NULL, NULL) < 0) {
        print("Failed to attach to init!\n");
        return;
    }
    wait(NULL);

    // Get address to inject to
    unsigned long data_inject_address = 0;
    read_init_map("00:00", &data_inject_address);

    // Get init text address
    unsigned long text_base = 0;
    read_init_map("00:01", &text_base);

    // Find the address of execve in the init text
    unsigned long execve_address = find_execve(text_base);
    if (!execve_address) {
        ptrace(PTRACE_DETACH, 1, NULL, NULL);
        return;
    }

    /*
     * Inject the data
     *
     * argv[0]         - pointer to "/init"
     * argv[1]/envp[0] - NULL
     * "/init"
     */
    ptrace(PTRACE_POKEDATA, 1, data_inject_address + (sizeof(void *) * 0),
           data_inject_address + 0x10);
    ptrace(PTRACE_POKEDATA, 1, data_inject_address + (sizeof(void *) * 1), 0);
#ifdef __aarch64__
    ptrace(PTRACE_POKEDATA, 1, data_inject_address + (sizeof(void *) * 2),
           *(unsigned long *)"/init\0\0\0");
#else
    ptrace(PTRACE_POKEDATA, 1, data_inject_address + (sizeof(void *) * 2),
           *(unsigned long *)"/ini");
    ptrace(PTRACE_POKEDATA, 1, data_inject_address + (sizeof(void *) * 3),
           *(unsigned long *)"t\0\0\0");
#endif

    // Get inits current registers
#ifdef __aarch64__
    struct iovec ioVec;
    struct user_pt_regs regs[1];
    ioVec.iov_base = regs;
    ioVec.iov_len = sizeof(*regs);
    if (ptrace(PTRACE_GETREGSET, 1, NT_PRSTATUS, &ioVec) < 0) {
        print("Failed to get registers (ARM64)!\n");
        ptrace(PTRACE_DETACH, 1, NULL, NULL);
        return;
    }
#else
    struct pt_regs regs;
    if (ptrace(PTRACE_GETREGS, 1, NULL, &regs) < 0) {
        print("Failed to get registers (ARM32)!\n");
        ptrace(PTRACE_DETACH, 1, NULL, NULL);
        return;
    }
#endif

    // Change the registers to call execve("/init", argv, envp)
#ifdef __aarch64__
    regs->regs[0] = data_inject_address + 0x10; /* char *filename */
    regs->regs[1] = data_inject_address + 0x00; /* char *argv[] */
    regs->regs[2] = data_inject_address + 0x08; /* char *envp[] */
    regs->pc = execve_address;
    if (ptrace(PTRACE_SETREGSET, 1, NT_PRSTATUS, &ioVec) < 0) {
        print("Failed to set registers (ARM64)!\n");
        ptrace(PTRACE_DETACH, 1, NULL, NULL);
        return;
    }
#else
    regs.ARM_r0 =
        data_inject_address + (sizeof(void *) * 2); /* char*  filename */
    regs.ARM_r1 = data_inject_address + (sizeof(void *) * 0); /* char** argp */
    regs.ARM_r2 = data_inject_address + (sizeof(void *) * 1); /* char** envp */
    regs.ARM_pc = execve_address;
    if (ptrace(PTRACE_SETREGS, 1, NULL, &regs) < 0) {
        print("Failed to set registers (ARM32)!\n");
        ptrace(PTRACE_DETACH, 1, NULL, NULL);
        return;
    }
#endif

    // Detach the ptrace
    ptrace(PTRACE_DETACH, 1, NULL, NULL);
}

int main(int argc, char **argv) {
    pthread_t thread_id;
    key_state_t key_state = {0, 0};
    struct stat sStat;

    if (pthread_create(&thread_id, NULL, key_listen, &key_state) != 0) {
        print("Failed to create key_listen thread!\n");
    }

    selinux_permissive("/sepolicy", "/dev/selinux", 1);

    boot_mode = get_boot_mode();
    print("Current boot mode: %d\n", boot_mode);

    if (key_state.volume_down_detected) {
        print("Trying to load ramdisk from secondary storage...\n");
        check_secondary_storage();
    }

    if (boot_mode == 2) {
        if (stat(SYSTEM_RECOVERY_RAMDISK, &sStat) == 0) {
            ramdisk_path = SYSTEM_RECOVERY_RAMDISK;
        }
    } else {
        if (stat(SYSTEM_NORMAL_RAMDISK, &sStat) == 0) {
            ramdisk_path = SYSTEM_NORMAL_RAMDISK;
        }
    }

    if (ramdisk_path != NULL) {
        extract_ramdisk(ramdisk_path);

        selinux_permissive("/sepolicy", "/sepolicy", 0);
        init_module(module_image, module_image_len, "");

        unmount("/system");
        unmount("/sys/fs/selinux");

        replace_init();
        return 0;
    }

    if (fork() == 0) {
        execl(DAEMONSU, DAEMONSU, "--auto-daemon", NULL);
    }

    return -1;
}
