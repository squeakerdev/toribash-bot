// decapAI bot, build B53: the chase lab's result: when its first move is played in place of the bot's own pick.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

if (cl.ran) {
	const Move cl_N = chosen;
	const bool cl_in = cl.res.scope[0] == 0;
	if (cl_in) cem_note |= 67108864;
	// [game access]
	else if (!cl_in) cl.p_status = "out of scope";
	else if (!cl.res.have || !decap_cl_digits_ok(cl.res.best.m1)) cl.p_status = "no lab sequence";
	else if (rescue_cem || fin.empty() || played_c < 0) cl.p_status = "no normal pick";
	else {
		Move cl_F;
		decap_cl_move(cl_N, cl.res.best.m1, cl_F);
		// [game access]
		else {
			const long cp0 = decap_ms_since(t0);
			const long cl_end = ko_full_ms * std::min(990, std::max(50, decap_env_int("DECAP_CL_P_END_PERMILLE", 950))) / 1000;
			decap_publish(chosen, pick.score, 3);
			const decap_full_t cl_sv_full = decap_full;
			const decap_diag_t cl_sv_diag = decap_diag;
			const bool cl_sv_xp = decap_xp.on;
			decap_xp.on = false;
			std::vector<Move> FN;
			FN.push_back(cl_F);
			FN.push_back(cl_N);
			std::vector<std::vector<Move>> scr(2);
			if (decap_cl_digits_ok(cl.res.best.m2)) {
				Move m2;
				decap_cl_move(cl_N, cl.res.best.m2, m2);
				scr[0].push_back(m2);
			}
			const std::vector<Move> *sets[3] = { &theirs, &held, &adv_seen };
			const unsigned seeds[3] = { plan_seed, plan_seed ^ 0x9e3779b9u, plan_seed ^ 0x3c6ef372u };
			bool safe = true, timed_out = false, n_all = true;
			double cF = 0, cN = 0, tF = 0, tN = 0, vF = 0, vN = 0;
			int nvF = 0, nvN = 0;
			bool n_safe = true;
			for (int s = 0; s < 3; s++) {
				if (sets[s]->empty()) continue;
				const long left = cl_end - decap_ms_since(t0);
				if (left <= 0) {
					timed_out = true;
					safe = false;
					break;
				}
				const int nt = (int)sets[s]->size();
				std::vector<decap_outcome> tmp;
				decap_plan_budget_ms = left;
				decap_plan_scripts = &scr;
				decap_plan_outcomes = &tmp;
				decap_lookahead(seat, FN, *sets[s], n_frames, skill_level, seeds[s]);
				decap_plan_outcomes = nullptr;
				decap_plan_scripts = nullptr;
				decap_plan_budget_ms = -1;
				int naF = 0, naN = 0;
				for (int t = 0; t < nt && (size_t)(nt + t) < tmp.size(); t++) {
					const decap_outcome &oF = tmp[t], &oN = tmp[nt + t];
					if (oF.ok == 1) {
						naF++;
						cl.p_nF++;
						const float dF = std::isfinite(oF.d2) ? oF.d2 : oF.d1;
						cF += first.d0 - dF;
						tF += oF.taken;
						if (oF.lost1 || oF.lme_plan) cl.p_lossF++;
					}
					if (oN.ok == 1) {
						naN++;
						cl.p_nN++;
						const float dN = std::isfinite(oN.d2) ? oN.d2 : oN.d1;
						cN += first.d0 - dN;
						tN += oN.taken;
					}
				}
				if (decap_full.ok(0)) vF += (double)decap_full.mean_v[0] * nt, nvF += nt;
				if (decap_full.ok(1)) vN += (double)decap_full.mean_v[1] * nt, nvN += nt;
				else n_safe = false;
				if (!(decap_full.ok(0) && naF == nt)) {
					safe = false;
					if (cl.p_why < 0) cl.p_why = decap_full.why.empty() ? 4 : decap_full.why[0];
					if (naF < nt && decap_ms_since(t0) >= cl_end) timed_out = true;
				}
				if (naN < nt) n_all = false;
			}
			decap_full = cl_sv_full;
			decap_diag = cl_sv_diag;
			decap_xp.on = cl_sv_xp;
			cl.p_ms = decap_ms_since(t0) - cp0;
			if (cl.p_nF > 0) cl.p_close_F = cF / cl.p_nF, cl.p_taken_F = tF / cl.p_nF;
			if (cl.p_nN > 0) cl.p_close_N = cN / cl.p_nN, cl.p_taken_N = tN / cl.p_nN;
			if (nvF > 0) cl.p_score_F = vF / nvF;
			if (nvN > 0) cl.p_score_N = vN / nvN;
			// [game access: 6 lines]
			else {
				cl.p_status = "PLAYED";
				cl.p_played = 1;
				ko_changed = true;
				cem_note |= 134217728;
				chosen = cl_F;
				int fc = -1;
				for (size_t c = 0; c < cand.size(); c++) if (memcmp(cand[c].states, cl_F.states, N_JOINTS) == 0) { fc = (int)c; break; }
				if (fc >= 0) played_c = fc;
				else {
					cand.push_back(cl_F);
					c_safe.push_back(1);
					c_mean.push_back(NAN);
					c_min.push_back(NAN);
					played_c = (int)cand.size() - 1;
				}
				cem_note &= ~1;
				if ((size_t)played_c >= mine.size() && fc >= 0 && !lp_is((size_t)played_c)) cem_note |= 1;
			}
			if (!cl.p_played) cem_note |= 268435456;
		}
	}
	auto cl_is_kept = [&](const Move &x) {
		for (const Move &r : cl.s_kept_first) if (memcmp(r.states, x.states, N_JOINTS) == 0) return true;
		return false;
	};
	for (size_t c = lp_c0; c < lp_c0 + lp_cn && c < cand.size(); c++) if (c_safe[c] && cl_is_kept(cand[c])) cl.s_safe++;
	for (size_t k = 0; k < fin.size(); k++) {
		if (!lp_is(fin[k]) || !cl_is_kept(cand[fin[k]])) continue;
		cl.s_fin++;
		if (fsafe[k]) cl.s_fin_safe++;
	}
	if (cl_is_kept(chosen)) cl.s_played_kept = 1;
	if (cl.s_played_kept) cem_note |= 536870912;
	char jp[N_JOINTS + 1];
	for (int j = 0; j < N_JOINTS; j++) jp[j] = (char)('0' + chosen.states[j]);
	jp[N_JOINTS] = 0;
	const decap_cl_res &R = cl.res;
	const bool lab_first = R.have && memcmp(jp, R.best.m1, N_JOINTS) == 0;
	char line[1600];
	snprintf(line, sizeof(line),
	         // [game access: 10 lines]
	decap_cl_log(line);
}
