// decapAI bot, build B53: the knockout stage (close range): a large pool scored on one-turn points; may override the pick.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

if (ko_on && !fin.empty() && played_c >= 0 && !rescue_cem) {
	const long ko_t_start = decap_ms_since(t0);
	const long ko_end = ko_full_ms * std::min(980, std::max(100, decap_env_int("DECAP_KO_END_PERMILLE", 770))) / 1000;
	const int ko_mut = std::min(4000, std::max(0, decap_env_int("DECAP_KO_MUT", 150)));
	const int ko_rand = std::min(4000, std::max(0, decap_env_int("DECAP_KO_RAND", 40)));
	const int ko_seeds = std::min(500, std::max(1, decap_env_int("DECAP_KO_SEEDS", 30)));
	const int ko_nrep = std::min(64, std::max(2, decap_env_int("DECAP_KO_REPLIES", 12)));
	const int ko_r1 = std::min(ko_nrep - 1, std::max(1, decap_env_int("DECAP_KO_R1", 4)));
	const int ko_keep = std::min(4000, std::max(1, decap_env_int("DECAP_KO_KEEP", 150)));
	const int ko_safe_k = std::min(64, std::max(1, decap_env_int("DECAP_KO_SAFE_K", 8)));
	const int ko_chunk = std::min(100000, std::max(16, decap_env_int("DECAP_KO_CHUNK", 1024)));
	const bool ko_log = decap_env_int("DECAP_KO_LOG", 0) != 0;
	const bool ko_dry = decap_env_int("DECAP_KO_DRY", 0) != 0;

	const double ko_lcb = std::max(0, decap_env_int("DECAP_KO_LCB_MILLI", 0)) / 1000.0;
	const int ko_fresh = std::min(16, std::max(0, decap_env_int("DECAP_KO_FRESH", 0)));
	const double ko_fresh_se = decap_env_int("DECAP_KO_FRESH_SE_MILLI", 1000) / 1000.0;

	const bool ko_rg_on = decap_rg_on();
	const float ko_rg_min = decap_env_int("DECAP_KO_RG_MIN_MILLI", 200) / 1000.0f;
	const float ko_rg_over = decap_env_int("DECAP_KO_RG_OVER_MILLI", 50) / 1000.0f;
	const float ko_rg_pts = ko_rg_on && decap_env_int("DECAP_KO_RG_CHARGE", 0) != 0 ? decap_rg_weight(seat) * 20000.0f : 0.0f;
	const float ko_rg_floor = decap_env_int("DECAP_RG_MIN_MILLI", 50) / 1000.0f;
	decap_publish(chosen, pick.score, 3);
	const decap_full_t ko_sv_full = decap_full;
	const decap_diag_t ko_sv_diag = decap_diag;
	const bool ko_sv_xp = decap_xp.on;
	decap_xp.on = false;
	const int ko_op = 1 - seat;
	auto ko_key = [](const Move &r) { return std::string((const char *)r.states, N_JOINTS); };

	std::vector<Move> kp;
	std::vector<int> kp_cand;
	std::vector<uint8_t> kp_src;
	std::set<std::string> kp_seen;
	auto kp_add = [&](const Move &r, int ci, uint8_t src) {
		if (!kp_seen.insert(ko_key(r)).second) return false;
		kp.push_back(r);
		kp_cand.push_back(ci);
		kp_src.push_back(src);
		return true;
	};
	kp_add(chosen, played_c, 0);
	std::vector<char> ko_finbad(cand.size(), 0);
	for (size_t k = 0; k < fin.size(); k++) if (!fsafe[k] && fin[k] < cand.size()) ko_finbad[fin[k]] = 1;

	std::map<std::string, int> ko_sc;
	std::set<std::string> ko_unsafe;
	for (size_t c = 0; c < cand.size(); c++) {
		if (c_safe[c] && !ko_finbad[c]) ko_sc.insert(std::make_pair(ko_key(cand[c]), (int)c));
		else ko_unsafe.insert(ko_key(cand[c]));
	}
	std::vector<size_t> ko_seed_c;
	{
		std::vector<size_t> fo;
		for (size_t k = 0; k < fin.size(); k++) if (fsafe[k]) fo.push_back(k);
		std::stable_sort(fo.begin(), fo.end(), [&](size_t a, size_t b) { return fscore[a] > fscore[b]; });
		for (size_t k : fo) if (std::find(ko_seed_c.begin(), ko_seed_c.end(), fin[k]) == ko_seed_c.end()) ko_seed_c.push_back(fin[k]);
		std::vector<size_t> co;
		for (size_t c = 0; c < cand.size(); c++) if (c_safe[c] && !ko_finbad[c]) co.push_back(c);
		std::stable_sort(co.begin(), co.end(), [&](size_t a, size_t b) { return stage_score(a) > stage_score(b); });
		for (size_t c : co) if (std::find(ko_seed_c.begin(), ko_seed_c.end(), c) == ko_seed_c.end()) ko_seed_c.push_back(c);
		if (ko_seed_c.size() > (size_t)ko_seeds) ko_seed_c.resize(ko_seeds);
	}
	for (size_t c : ko_seed_c) kp_add(cand[c], (int)c, 1);
	auto ko_mutate = [&](Move r, int who) {
		int k = 1 + rand() % 3, done = 0;
		bool used[N_JOINTS] = {};
		for (int tries = 0; done < k && tries < 64; tries++) {
			int j = rand() % N_JOINTS;
			// [game access]
			used[j] = true;
			int st = 1 + rand() % 3;
			if (st >= r.states[j]) st++;
			r.states[j] = (uint8_t)st;
			done++;
		}
		return r;
	};
	int ko_n_mut = 0, ko_n_rand = 0;
	if (!ko_seed_c.empty()) {
		for (int t = 0; ko_n_mut < ko_mut && t < ko_mut * 20; t++) {
			const Move m = ko_mutate(cand[ko_seed_c[t % ko_seed_c.size()]], seat);
			const std::string k = ko_key(m);
			if (ko_unsafe.count(k)) continue;
			std::map<std::string, int>::const_iterator it = ko_sc.find(k);
			ko_n_mut += it != ko_sc.end() ? kp_add(m, it->second, 1) : kp_add(m, -1, 2);
		}
	}
	for (size_t c = 0; c < cand.size(); c++) if (c_safe[c] && !ko_finbad[c]) kp_add(cand[c], (int)c, 1);
	size_t ko_n_search = 0;
	for (size_t i = 1; i < kp.size(); i++) ko_n_search += kp_src[i] == 1;
	for (int t = 0; ko_n_rand < ko_rand && t < ko_rand * 20; t++) {
		Move r = chosen;
		// [game access]
		if (ko_unsafe.count(ko_key(r)) || ko_sc.count(ko_key(r))) continue;
		ko_n_rand += kp_add(r, -1, 3);
	}

	std::vector<Move> kr, ko_guess;
	std::set<std::string> kr_seen;
	for (const Move &r : theirs) ko_guess.push_back(r);
	for (const Move &r : held) ko_guess.push_back(r);
	for (const Move &r : adv_seen) ko_guess.push_back(r);
	Move ko_hold = theirs[0];
	for (int j = 0; j < N_JOINTS; j++) ko_hold.states[j] = STATE_HOLD;
	auto kr_add = [&](const Move &r) {
		if ((int)kr.size() >= ko_nrep || !kr_seen.insert(ko_key(r)).second) return;
		kr.push_back(r);
	};
	for (const Move &r : theirs) kr_add(r);
	if ((int)kr.size() >= ko_nrep && !kr_seen.count(ko_key(ko_hold))) {
		kr_seen.erase(ko_key(kr.back()));
		kr.pop_back();
	}
	kr_add(ko_hold);
	for (const Move &r : held) kr_add(r);
	for (const Move &r : adv_seen) kr_add(r);
	for (int t = 0; (int)kr.size() < ko_nrep && t < 2000; t++) kr_add(ko_mutate(ko_guess[t % ko_guess.size()], ko_op));
	const int nr = (int)kr.size(), nr1 = std::min(ko_r1, nr - 1);

	long ko_ms_r1 = 0, ko_ms_r2 = 0, ko_ms_safe = 0;
	int ko_sims = 0, ko_noans = 0;

	int ko_disp = std::min(20, std::max(0, decap_env_int("DECAP_KO_DISPATCH", 20)));
	{
		cpu_set_t ko_cs;
		CPU_ZERO(&ko_cs);
		if (sched_getaffinity(0, sizeof(ko_cs), &ko_cs) == 0 && CPU_COUNT(&ko_cs) > 0) ko_disp = std::min(ko_disp, (int)CPU_COUNT(&ko_cs));
	}
	int ko_disp_fail = 0;
	bool ko_serial = false;
	auto ko_call = [&](const std::vector<Move> &sub, const std::vector<Move> &R, int turn_only, unsigned seed,
	                   long left, std::vector<decap_outcome> &tmp) {
		const size_t nm = sub.size(), nrr = R.size(), n = nm * nrr;
		const int W = (int)std::min((size_t)ko_disp, nm);
		decap_outcome *shm = nullptr;
		const size_t map_n = n * sizeof(decap_outcome) + 256 * sizeof(int32_t);
		if (W >= 2 && !ko_serial) {
			// [game access: 2 lines]
		}
		if (!shm) {
			decap_plan_turn_only = turn_only;
			decap_plan_budget_ms = left;
			decap_plan_outcomes = &tmp;

			std::map<std::string, decap_rg_stat> ko_sv_rg;
			const bool ko_rg_keep = ko_rg_on && turn_only != 0;
			if (ko_rg_keep) ko_sv_rg = decap_rg_seen;
			decap_lookahead(seat, sub, R, n_frames, skill_level, seed);
			if (ko_rg_keep) decap_rg_seen.swap(ko_sv_rg);
			decap_plan_outcomes = nullptr;
			decap_plan_budget_ms = -1;
			decap_plan_turn_only = 0;
			return;
		}
		memset((void *)shm, 0, map_n);
		volatile int32_t *done = (volatile int32_t *)((char *)shm + n * sizeof(decap_outcome));
		struct timespec td;
		clock_gettime(CLOCK_MONOTONIC, &td);
		std::vector<pid_t> kids;
		std::vector<int> kid_w;
		for (int w = 0; w < W; w++) {
			const size_t lo = nm * w / W, hi = nm * (w + 1) / W;
			if (hi <= lo) continue;
			// [game access]
			if (pid == 0) {
				// [game access: 6 lines]
				setenv("DECAP_LOOKAHEAD_PARALLEL", "1", 1);
				std::vector<Move> part(sub.begin() + lo, sub.begin() + hi);
				std::vector<decap_outcome> res;
				decap_plan_turn_only = turn_only;
				decap_plan_budget_ms = std::max(1L, left - decap_ms_since(td));
				decap_plan_outcomes = &res;
				decap_lookahead(seat, part, R, n_frames, skill_level, seed);
				const size_t k = std::min(res.size(), (hi - lo) * nrr);
				if (k) memcpy((void *)(shm + lo * nrr), res.data(), k * sizeof(decap_outcome));
				__sync_synchronize();
				done[w] = 1;
				// [game access]
			}
			if (pid > 0) {
				kids.push_back(pid);
				kid_w.push_back(w);
			}
			else ko_disp_fail++;
		}

		size_t alive = kids.size();
		while (alive > 0) {
			for (size_t q = 0; q < kids.size(); q++) {
				pid_t &k = kids[q];
				if (k <= 0) continue;
				__sync_synchronize();
				if (done[kid_w[q]]) {
					// [game access]
					k = -1;
					alive--;
				}
				// [game access]
					k = -1;
					alive--;
					ko_disp_fail++;
				}
			}
			if (alive == 0) break;
			if (decap_ms_since(td) > left + 1000) {
				for (pid_t &k : kids) {
					if (k > 0) {
						// [game access: 2 lines]
						k = -1;
						ko_disp_fail++;
					}
				}
				break;
			}
			usleep(500);
		}
		tmp.assign(shm, shm + n);
		// [game access]
	};

	auto ko_run = [&](const std::vector<Move> &M, const std::vector<Move> &R, int turn_only, unsigned seed,
	                  std::vector<decap_outcome> &out, int chunk, long soft_end, size_t *ran) {
		out.assign(M.size() * R.size(), decap_outcome());
		for (decap_outcome &o : out) o.ok = 0;
		const size_t per = std::max((size_t)1, (size_t)chunk / R.size());
		if (ran) *ran = 0;
		for (size_t s = 0; s < M.size(); s += per) {
			long left = ko_end - decap_ms_since(t0);
			if (left <= 0) return false;
			if (soft_end >= 0 && s > 0 && decap_ms_since(t0) >= soft_end) break;
			if (ran) *ran = std::min(M.size(), s + per);
			std::vector<Move> sub(M.begin() + s, M.begin() + std::min(M.size(), s + per));
			std::vector<decap_outcome> tmp;
			ko_call(sub, R, turn_only, seed, left, tmp);
			for (size_t i = 0; i < tmp.size() && (s * R.size() + i) < out.size(); i++) {
				out[s * R.size() + i] = tmp[i];
				ko_sims++;
				ko_noans += tmp[i].ok != 1;
			}
		}
		return decap_ms_since(t0) < ko_end;
	};
	const char *ko_status = "complete";
	std::vector<float> s1(kp.size(), NAN), s2(kp.size(), NAN);
	std::vector<char> kdq(kp.size(), 0);
	std::vector<float> ko_pr(kp.size() * (size_t)std::max(1, nr), NAN);
	std::vector<double> ko_rgs(kp.size(), 0.0);
	std::vector<int> ko_rgn(kp.size(), 0);
	auto ko_risk = [&](size_t i) { return ko_rgn[i] > 0 ? (float)(ko_rgs[i] / ko_rgn[i]) : 0.0f; };
	int ko_veto_n = 0, ko_veto_first = -1;
	std::vector<size_t> surv, rank2;
	int ko_choice = 0, ko_pick_rank = -1, ko_judged = 0, ko_rejected = 0, ko_keep_eff = 0;
	int ko_passed = -1, ko_fresh_n = 0;
	double ko_w_mean = NAN, ko_w_se = NAN, ko_w_lb = NAN, ko_fresh_d = NAN, ko_fresh_dse = NAN;
	const char *ko_dec = "fallback";
	size_t ko_r1_ran = 0;
	bool ko_fin_forced = true;
	bool ko_ok = nr >= 2 && decap_ms_since(t0) < ko_end;
	// [game access]
	if (ko_ok) {
		std::vector<Move> R1(kr.begin(), kr.begin() + nr1);
		std::vector<decap_outcome> o1;
		const long ta = decap_ms_since(t0);

		const long r1_soft = ta + (ko_end - ta) * std::min(950, std::max(100, decap_env_int("DECAP_KO_R1_SHARE_PERMILLE", 550))) / 1000;
		ko_ok = ko_run(kp, R1, 1, plan_seed ^ 0x4b4f3101u, o1, std::min(ko_chunk, std::max(16, decap_env_int("DECAP_KO_R1_CHUNK", 800))), r1_soft, &ko_r1_ran);
		ko_ms_r1 = decap_ms_since(t0) - ta;
		// [game access]
		for (size_t i = 0; i < kp.size() && ko_ok; i++) {
			double sum = 0.0;
			int n = 0;
			for (int r = 0; r < nr1; r++) {
				const decap_outcome &o = o1[i * nr1 + r];
				if (o.ok != 1 || !std::isfinite(o.dealt1) || !std::isfinite(o.taken1)) continue;
				if (ko_rg_pts > 0.0f) {
					const float pts = (o.dealt1 - o.taken1) - (o.rg1 > ko_rg_floor ? ko_rg_pts * (o.rg1 - ko_rg_floor) : 0.0f);
					sum += pts;
					ko_pr[i * nr + r] = pts;
				}
				else {
					sum += o.dealt1 - o.taken1;
					ko_pr[i * nr + r] = o.dealt1 - o.taken1;
				}
				n++;
				if (o.lost1) kdq[i] = 1;
				if (ko_rg_on) {
					ko_rgs[i] += o.lost1 ? 1.0 : (double)o.rg1;
					ko_rgn[i]++;
				}
			}
			if (n == nr1) s1[i] = (float)(sum / n);
		}
		if (ko_ok) {
			std::vector<size_t> ord;
			for (size_t i = 0; i < kp.size(); i++) if (std::isfinite(s1[i])) ord.push_back(i);
			std::stable_sort(ord.begin(), ord.end(), [&](size_t a, size_t b) { return s1[a] > s1[b]; });
			std::vector<char> in(kp.size(), 0);

			const double rate = ko_ms_r1 > 0 ? (double)(nr1 * ko_r1_ran) / (double)ko_ms_r1 : 1.0;
			const long reserve = (long)(3.0 * ko_safe_k * theirs.size() / std::max(rate, 1e-3)) + 200;
			const long avail = ko_end - decap_ms_since(t0) - reserve;
			const double slack = std::min(1000, std::max(100, decap_env_int("DECAP_KO_R2_MARGIN_PERMILLE", 800))) / 1000.0;
			const int fit = avail > 0 ? (int)(avail * rate * slack / std::max(1, nr - nr1)) : 0;
			int n_fin = 0;
			for (size_t i = 1; i < kp.size(); i++) {
				n_fin += kp_cand[i] >= 0 && std::isfinite(s1[i]) && std::find(fin.begin(), fin.end(), (size_t)kp_cand[i]) != fin.end();
			}
			ko_fin_forced = fit >= 1 + n_fin + 10;
			int n_forced = 0;
			for (size_t i = 0; i < kp.size(); i++) {
				if (!std::isfinite(s1[i])) continue;
				if (i == 0 || (ko_fin_forced && kp_cand[i] >= 0 && std::find(fin.begin(), fin.end(), (size_t)kp_cand[i]) != fin.end())) {
					in[i] = 1;
					n_forced++;
				}
			}
			ko_keep_eff = std::max(std::min(5, ko_keep), std::min(ko_keep, fit - n_forced));
			for (size_t k = 0, got = 0; k < ord.size() && (int)got < ko_keep_eff; k++) {
				if (!in[ord[k]]) {
					in[ord[k]] = 1;
					got++;
				}
			}
			for (size_t i = 0; i < kp.size(); i++) if (in[i] && std::isfinite(s1[i])) surv.push_back(i);
			std::vector<Move> M2, R2(kr.begin() + nr1, kr.end());
			for (size_t i : surv) M2.push_back(kp[i]);
			std::vector<decap_outcome> o2;
			const long tb = decap_ms_since(t0);
			ko_ok = !M2.empty() && ko_run(M2, R2, 1, plan_seed ^ 0x4b4f3102u, o2, ko_chunk, -1, nullptr);
			ko_ms_r2 = decap_ms_since(t0) - tb;
			// [game access]
			const int nr2 = nr - nr1;
			for (size_t q = 0; q < surv.size() && ko_ok; q++) {
				size_t i = surv[q];
				double sum = 0.0;
				int n = 0;
				for (int r = 0; r < nr2; r++) {
					const decap_outcome &o = o2[q * nr2 + r];
					if (o.ok != 1 || !std::isfinite(o.dealt1) || !std::isfinite(o.taken1)) continue;
					if (ko_rg_pts > 0.0f) {
						const float pts = (o.dealt1 - o.taken1) - (o.rg1 > ko_rg_floor ? ko_rg_pts * (o.rg1 - ko_rg_floor) : 0.0f);
						sum += pts;
						ko_pr[i * nr + nr1 + r] = pts;
					}
					else {
						sum += o.dealt1 - o.taken1;
						ko_pr[i * nr + nr1 + r] = o.dealt1 - o.taken1;
					}
					n++;
					if (o.lost1) kdq[i] = 1;
					if (ko_rg_on) {
						ko_rgs[i] += o.lost1 ? 1.0 : (double)o.rg1;
						ko_rgn[i]++;
					}
				}
				if (n == nr2) s2[i] = (float)((sum + (double)s1[i] * nr1) / nr);
			}
		}
	}
	if (ko_ok) {

		bool any_clean = false;
		for (size_t i : surv) any_clean |= std::isfinite(s2[i]) && !kdq[i];
		for (size_t i : surv) if (std::isfinite(s2[i]) && (!any_clean || !kdq[i])) rank2.push_back(i);
		std::stable_sort(rank2.begin(), rank2.end(), [&](size_t a, size_t b) { return s2[a] > s2[b]; });
		for (size_t k = 0; k < rank2.size(); k++) if (rank2[k] == 0) ko_pick_rank = (int)k + 1;
		auto ko_se_of = [&](size_t i) {
			double m = 0.0, v = 0.0;
			for (int q = 0; q < nr; q++) m += ko_pr[i * nr + q];
			m /= nr;
			for (int q = 0; q < nr; q++) v += (ko_pr[i * nr + q] - m) * (ko_pr[i * nr + q] - m);
			return nr > 1 ? sqrt(v / (nr - 1) / nr) : 0.0;
		};
		std::vector<size_t> ko_order = rank2;
		if (ko_lcb > 0.0) {
			ko_order.clear();
			ko_passed = 0;
			if (std::isfinite(s2[0])) {
				for (size_t i : rank2) {
					if (i == 0) continue;
					const double lb = s2[i] - ko_lcb * ko_se_of(i);
					if (lb > s2[0]) {
						ko_order.push_back(i);
						ko_passed++;
					}
				}
			}
		}

		std::vector<size_t> nj;
		for (size_t i : ko_order) if (kp_cand[i] < 0 && (int)nj.size() < ko_safe_k) nj.push_back(i);
		std::vector<char> jsafe(kp.size(), 0), jdone(kp.size(), 0);
		if (!nj.empty() && decap_ms_since(t0) < ko_end) {
			std::vector<Move> MJ;
			for (size_t i : nj) MJ.push_back(kp[i]);
			std::vector<decap_outcome> oj;
			const long tc = decap_ms_since(t0);
			ko_serial = true;
			ko_run(MJ, theirs, 0, plan_seed, oj, ko_chunk, -1, nullptr);
			ko_serial = false;
			ko_ms_safe = decap_ms_since(t0) - tc;
			for (size_t q = 0; q < nj.size(); q++) {
				bool all = true;
				for (size_t r = 0; r < theirs.size(); r++) all = all && oj[q * theirs.size() + r].ok == 1;
				jdone[nj[q]] = 1;
				jsafe[nj[q]] = all && decap_full.ok(q);
				ko_judged++;
				ko_rejected += !jsafe[nj[q]];
			}
		}
		ko_choice = -1;
		for (size_t i : ko_order) {
			if (kp_cand[i] >= 0 || (jdone[i] && jsafe[i])) {

				if (ko_rg_on && i != 0 && ko_risk(i) >= ko_rg_min && ko_risk(i) - ko_risk(0) > ko_rg_over) {
					if (ko_veto_first < 0) ko_veto_first = (int)i;
					ko_veto_n++;
					continue;
				}
				ko_choice = (int)i;
				break;
			}
		}
		if (ko_choice < 0) {
			ko_choice = 0;
			// [game access: 2 lines]
		}
		// [game access]
		if (ko_veto_n > 0) cem_note |= 32768;
		if (ko_choice > 0) {
			ko_w_mean = s2[ko_choice];
			ko_w_se = ko_se_of((size_t)ko_choice);
			ko_w_lb = ko_w_mean - ko_lcb * ko_w_se;
		}
		if (ko_choice > 0 && ko_fresh > 0) {
			std::vector<Move> FR;
			std::set<std::string> fseen = kr_seen;
			for (const Move &g : ko_guess) {
				if ((int)FR.size() >= ko_fresh) break;
				if (fseen.insert(ko_key(g)).second) FR.push_back(g);
			}
			for (int t = 0; (int)FR.size() < ko_fresh && t < 2000; t++) {
				Move m = ko_mutate(ko_guess[t % ko_guess.size()], ko_op);
				if (fseen.insert(ko_key(m)).second) FR.push_back(m);
			}
			std::vector<Move> MF;
			MF.push_back(kp[ko_choice]);
			MF.push_back(kp[0]);
			std::vector<decap_outcome> of;
			bool fok = !FR.empty() && ko_run(MF, FR, 1, plan_seed ^ 0x4b4f3103u, of, ko_chunk, -1, nullptr);
			std::vector<double> dd;
			for (size_t q = 0; fok && q < FR.size(); q++) {
				const decap_outcome &ow = of[q], &on = of[FR.size() + q];
				if (ow.ok != 1 || on.ok != 1 || !std::isfinite(ow.dealt1) || !std::isfinite(on.dealt1)) { fok = false; break; }
				if (ko_rg_pts > 0.0f) {
					const float cw = ow.rg1 > ko_rg_floor ? ko_rg_pts * (ow.rg1 - ko_rg_floor) : 0.0f, cn = on.rg1 > ko_rg_floor ? ko_rg_pts * (on.rg1 - ko_rg_floor) : 0.0f;
					dd.push_back(((ow.dealt1 - ow.taken1) - cw) - ((on.dealt1 - on.taken1) - cn));
				}
				else dd.push_back((ow.dealt1 - ow.taken1) - (on.dealt1 - on.taken1));
			}
			ko_fresh_n = (int)dd.size();
			if (fok && !dd.empty()) {
				double m = 0.0, v = 0.0;
				for (double x : dd) m += x;
				m /= dd.size();
				for (double x : dd) v += (x - m) * (x - m);
				ko_fresh_d = m;
				ko_fresh_dse = dd.size() > 1 ? sqrt(v / (dd.size() - 1) / dd.size()) : 0.0;
			}
			if (!fok || !(ko_fresh_d > ko_fresh_se * ko_fresh_dse)) {
				// [game access]
				ko_choice = 0;
			}
		}
		if (ko_choice > 0) ko_dec = "override";
		else if (!strcmp(ko_dec, "fallback")) ko_dec = "normal pick best";
		cem_note |= 4096;
	}
	else {
		ko_choice = 0;
	}
	decap_full = ko_sv_full;
	decap_diag = ko_sv_diag;
	decap_xp.on = ko_sv_xp;
	const int ko_would = ko_choice;
	if (ko_dry) ko_choice = 0;
	if (ko_choice != 0) {
		ko_changed = true;
		cem_note |= 8192;
		chosen = kp[ko_choice];
		if (kp_cand[ko_choice] >= 0) {
			played_c = kp_cand[ko_choice];
		}
		else {
			cand.push_back(kp[ko_choice]);
			c_safe.push_back(1);
			c_mean.push_back(NAN);
			c_min.push_back(NAN);
			played_c = (int)cand.size() - 1;
		}
		cem_note &= ~1;
		if ((size_t)played_c >= mine.size() && kp_cand[ko_choice] >= 0 && !lp_is((size_t)played_c)) cem_note |= 1;
	}
	DECAP_LF_KO();
	if (ko_log) {
		static const char *srcn[4] = { "normal pick", "search candidate", "mutation", "random" };
		char line[1800];
		snprintf(line, sizeof(line),
		         // [game access: 7 lines]
		{
			size_t L = strlen(line);
			if (L > 0 && line[L - 1] == 10) line[--L] = 0;
			// [game access: 2 lines]
		}
		{
			char jn[N_JOINTS + 1], jw[N_JOINTS + 1];
			for (int j = 0; j < N_JOINTS; j++) {
				jn[j] = (char)('0' + kp[0].states[j]);
				jw[j] = (char)('0' + kp[ko_would].states[j]);
			}
			jn[N_JOINTS] = jw[N_JOINTS] = 0;
			size_t L = strlen(line);
			if (L > 0 && line[L - 1] == 10) line[--L] = 0;
			// [game access: 2 lines]
		}
		if (ko_rg_on) {
			char jv[N_JOINTS + 1];
			for (int j = 0; j < N_JOINTS; j++) jv[j] = (char)('0' + (ko_veto_first >= 0 ? kp[ko_veto_first].states[j] : 0));
			jv[N_JOINTS] = 0;
			size_t L = strlen(line);
			if (L > 0 && line[L - 1] == 10) line[--L] = 0;
			// [game access: 3 lines]
		}
		fputs(line, stderr);
		fflush(stderr);
		const char *dir = getenv("DECAP_DECISION_LOG");
		if (dir && *dir) {
			char path[600];
			// [game access]
			int fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 0644);
			if (fd >= 0) {
				ssize_t w = write(fd, line, strlen(line));
				(void)w;
				close(fd);
			}
		}
	}
}
