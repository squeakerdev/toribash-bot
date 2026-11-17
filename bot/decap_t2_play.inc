// decapAI bot, build B53: the chase search (new in B53): when it runs and when its move replaces the bot's own pick.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

if (t2g.on && t2g.open) {
	decap_t2g_restore(t2g);
	const long t2_at = decap_ms_since(t0);
	const Move t2_N = chosen;
	const std::string t2_nd = decap_t2_dig(chosen.states);
	const long t2_left = t2g.clock ? t2g.end_ms - t2_at : -1;
	const char *t2_status = "";
	int t2_ran = 0, t2_fin = 0, t2_played = 0;
	decap_t2_res t2r;
	memset(&t2r, 0, sizeof(t2r));
	// [game access]
	const int t2_threads = std::max(1, std::min(64, decap_env_int("DECAP_T2G_THREADS", 8)));
	std::vector<std::string> t2_s1, t2_s2;
	std::vector<std::pair<std::string, std::string>> t2_pol, t2_rep;
	float t2_pr[2][N_JOINTS][4];
	int t2_have_pr = 0;
	long t2_ms = 0;
	if (t2g.clock && t2_left < decap_env_int("DECAP_T2G_MIN_LEFT_MS", 1500)) t2_status = "no time left";
	else {
		std::vector<std::string> cs;
		for (const decap_dec_cand &c : decap_last_dec.cands) cs.push_back(decap_t2_dig(c.joints));
		t2_s1.push_back(decap_t2_dig(prev));
		t2_s1.push_back(t2_nd);
		for (const std::string &s : cs) t2_s1.push_back(s);
		t2_s2.push_back(t2_nd);
		for (const std::string &s : cs) t2_s2.push_back(s);
		auto keep_ok = [](std::vector<std::string> &v) {
			std::vector<std::string> o;
			for (const std::string &s : v) if (decap_t2_ok(s)) o.push_back(s);
			v.swap(o);
		};
		keep_ok(t2_s1);
		keep_ok(t2_s2);
		std::mt19937_64 t2_rng(t2_seed ^ 0x706f6cull);
		std::vector<std::string> mine_p, his_p;
		// [game access]
			t2_have_pr = 1;
			decap_t2_draws(t2_pr[0], 32, t2_rng, mine_p);
			decap_t2_draws(t2_pr[1], 4, t2_rng, his_p);
		}
		for (const std::string &m : mine_p) t2_pol.push_back(std::make_pair(m, m));
		uint8_t hp[N_JOINTS];
		// [game access]
		const std::string hold(N_JOINTS, '3'), relax(N_JOINTS, '4'), hprev = decap_t2_dig(hp);
		t2_rep.push_back(std::make_pair(hold, hold));
		t2_rep.push_back(std::make_pair(relax, relax));
		t2_rep.push_back(std::make_pair(hprev, hprev));
		for (const std::string &m : his_p) t2_rep.push_back(std::make_pair(m, m));
		std::string b1, b2, bp, br;
		for (const std::string &s : t2_s1) b1 += s;
		for (const std::string &s : t2_s2) b2 += s;
		for (const auto &s : t2_pol) bp += s.first + s.second;
		for (const auto &s : t2_rep) br += s.first + s.second;
		// [game access: 2 lines]
		decap_t2_req q;
		memset(&q, 0, sizeof(q));
		q.H = t2h.get();
		q.me = seat;
		q.seed = t2_seed;
		q.threads = t2_threads;
		q.budget_ms = t2g.clock ? t2_left : -1;
		q.seeds1 = b1.c_str(); q.n_seeds1 = (int)t2_s1.size();
		q.seeds2 = b2.c_str(); q.n_seeds2 = (int)t2_s2.size();
		q.pol = bp.c_str(); q.n_pol = (int)t2_pol.size();
		q.rep = br.c_str(); q.n_rep = (int)t2_rep.size();
		q.w_close = 1000.0;
		q.nlab = 48;
		q.pools = "abc";
		t2_ran = 1;
		decap_t2_decide(q, t2r);
		t2_ms = decap_ms_since(t0) - t2_at;
		t2r.err[sizeof(t2r.err) - 1] = 0;
		t2r.lab_scope[sizeof(t2r.lab_scope) - 1] = 0;
		t2r.m1[sizeof(t2r.m1) - 1] = 0;
		t2r.m2[sizeof(t2r.m2) - 1] = 0;
		const bool in_time = !t2g.clock || decap_ms_since(t0) <= t2g.end_ms;
		// [game access: 4 lines]
		else {
			t2_fin = 1;
			t2_played = 1;
			t2_status = "PLAYED";
			decap_cl_move(t2_N, t2r.m1, chosen);
			decap_publish(chosen, pick.score, 3);
			decap_last_dec.played = chosen;
			decap_last_dec.cem_note |= 1073741824;
			int fc = -1;
			for (size_t c = 0; c < decap_last_dec.cands.size(); c++) if (memcmp(decap_last_dec.cands[c].joints, chosen.states, N_JOINTS) == 0) { fc = (int)c; break; }
			if (fc < 0) {
				decap_dec_cand d;
				memcpy(d.joints, chosen.states, N_JOINTS);
				d.grip[0] = chosen.grip[0];
				d.grip[1] = chosen.grip[1];
				d.safe = 1;
				d.tag = 14;
				d.mean_v = NAN;
				d.min_v = NAN;
				d.fscore = NAN;
				if (decap_last_dec.cands.size() < (size_t)DECAP_DIAG_MAX) decap_last_dec.cands.push_back(d);
				else if (!decap_last_dec.cands.empty()) decap_last_dec.cands.back() = d;
				fc = decap_last_dec.cands.empty() ? -1 : (int)decap_last_dec.cands.size() - 1;
			}
			decap_last_dec.played_idx = fc;
		}
		decap_t2_dump(seat, t2_seed, t2_threads, t2_s1, t2_s2, t2_pol, t2_rep, t2_pr, t2_have_pr, t2r, t2g.gap, t2_ms);
	}
	char line[1400];
	snprintf(line, sizeof(line),
	         // [game access: 7 lines]
	decap_t2_log(line);
}
decap_t2g_restore(t2g);
