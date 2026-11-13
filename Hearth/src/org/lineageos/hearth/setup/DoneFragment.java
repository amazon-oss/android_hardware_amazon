/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.hearth.setup;

import android.os.Bundle;
import android.support.v17.leanback.app.GuidedStepFragment;
import android.support.v17.leanback.widget.GuidanceStylist;
import android.support.v17.leanback.widget.GuidedAction;

import java.util.List;

import org.lineageos.hearth.R;

public class DoneFragment extends GuidedStepFragment {

    private static final long ACTION_FINISH = 1;

    @Override
    public GuidanceStylist.Guidance onCreateGuidance(Bundle savedInstanceState) {
        return new GuidanceStylist.Guidance(getString(R.string.setup_done_title),
                getString(R.string.setup_done_summary), getString(R.string.setup_breadcrumb),
                getActivity().getDrawable(R.drawable.ic_hearth_mark));
    }

    @Override
    public void onCreateActions(List<GuidedAction> actions, Bundle savedInstanceState) {
        actions.add(new GuidedAction.Builder(getActivity())
                .id(ACTION_FINISH)
                .title(R.string.setup_finish)
                .build());
    }

    @Override
    public void onGuidedActionClicked(GuidedAction action) {
        if (action.getId() == ACTION_FINISH) {
            ((SetupActivity) getActivity()).complete();
        }
    }
}
