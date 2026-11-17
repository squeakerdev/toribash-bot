// decapAI bot, build B53: the entry planner (2.5 to 3.5 m apart): a search over two-move sequences.
//
// Original comments are removed. "// [game access]" stands for statements that read or call the game or manage the
// processes the search runs in; "[REDACTED]" for a function that mainly does so. The game's own names are replaced by
// neutral ones. This file does not compile.

// [game access: 2 lines]
	const long en_t_start = decap_ms_since(t0);
	const long en_cem_end = ko_full_ms * std::min(980, std::max(50, decap_env_int("DECAP_ENTRY_END_PERMILLE", 900))) / 1000;
	const long en_safe_end = ko_full_ms * std::min(980, std::max(50, decap_env_int("DECAP_ENTRY_SAFE_END_PERMILLE", 950))) / 1000;
	const int en_pop = std::min(2000, std::max(4, decap_env_int("DECAP_ENTRY_POP", 80)));
	const int en_gens = std::min(8, std::max(1, decap_env_int("DECAP_ENTRY_GENS", 5)));
	const int en_elites = std::min(en_pop, std::max(1, decap_env_int("DECAP_ENTRY_ELITES", 10)));
	const double en_p0 = decap_env_int("DECAP_ENTRY_P0_MILLI", 400) / 1000.0, en_mix = decap_env_int("DECAP_ENTRY_MIX_MILLI", 700) / 1000.0;
	const double en_floor = decap_env_int("DECAP_ENTRY_FLOOR_MILLI", 30) / 1000.0;
	const double EN_CAP = 15000.0, EN_LOSS = -15000.0;
	const bool en_log = decap_env_int("DECAP_ENTRY_LOG", 0) != 0;

	const int en_min_gens = std::max(0, decap_env_int("DECAP_ENTRY_MIN_GENS", 0));
	const int en_op = 1 - seat;
	decap_publish(chosen, pick.score, 3);
	const Move en_N = chosen;
	decap_en_lf_t &EN = decap_en_lf.h;
	memset(&EN, 0, sizeof(EN));
	EN.hdr_size = (uint16_t)sizeof(EN);
	EN.version = 3;
	EN.min_gens = en_min_gens;
	EN.d0 = first.d0;
	// [game access]
	EN.pop = en_pop;
	EN.gens_cfg = en_gens;
	EN.elites = en_elites;
	EN.ms_start = (int32_t)en_t_start;
	EN.ms_cem_end = (int32_t)std::min(en_cem_end, 2000000000L);
	EN.ms_safe_end = (int32_t)std::min(en_safe_end, 2000000000L);
	EN.J_best = EN.J_NN = EN.J_Nbest = EN.J_F_N2 = EN.f_score = NAN;
	EN.n_score = stage_score((size_t)played_c);
	EN.f_cand = -1;
	for (int g = 0; g < 8; g++) EN.J_gen[g] = NAN;
	for (int s = 0; s < 3; s++) EN.why[s] = 255;
	memcpy(EN.N, en_N.states, N_JOINTS);
	decap_en_lf.have = true;
	auto en_key = [](const Move &r) { return std::string((const char *)r.states, N_JOINTS); };

	std::vector<Move> en_R;
	{
		std::set<std::string> s;
		for (const Move &r : theirs) if (s.insert(en_key(r)).second) en_R.push_back(r);
		for (const Move &r : held) if (s.insert(en_key(r)).second) en_R.push_back(r);
	}
	const size_t en_nr = en_R.size();
	EN.n_rep = (int32_t)en_nr;

	int en_disp = std::min(20, std::max(1, decap_env_int("DECAP_ENTRY_DISPATCH", 20)));
	{
		cpu_set_t cs;
		CPU_ZERO(&cs);
		if (sched_getaffinity(0, sizeof(cs), &cs) == 0 && CPU_COUNT(&cs) > 0) en_disp = std::min(en_disp, (int)CPU_COUNT(&cs));
	}
	EN.disp = en_disp;
	std::vector<std::pair<Move, Move>> en_seq;
	std::vector<double> en_val;
	std::map<std::string, int> en_idx;
	int en_turns = 0;

	auto en_run = [&](const std::vector<int> &qs, long end, long *ms_out, int *turns_out) -> int {
		std::map<std::string, std::vector<int>> by_m1;
		std::vector<std::string> m1_order;
		for (int q : qs) {
			const std::string k = en_key(en_seq[q].first);
			if (!by_m1.count(k)) m1_order.push_back(k);
			by_m1[k].push_back(q);
		}
		const size_t n_slot = qs.size() * en_nr;
		std::vector<size_t> pos(en_seq.size(), 0);
		for (size_t i = 0; i < qs.size(); i++) pos[qs[i]] = i;
		const size_t map_n = n_slot * sizeof(decap_en_out) + 256 * sizeof(int32_t);
		// [game access: 2 lines]
		decap_en_out *shm = (decap_en_out *)mp;
		memset(mp, 0, map_n);
		volatile int32_t *done = (volatile int32_t *)((char *)mp + n_slot * sizeof(decap_en_out));
		int32_t *qnext = (int32_t *)done + 250;
		struct grp { int q0; size_t r; std::vector<int> qs; };
		std::vector<grp> G;
		int turns = 0;
		const bool t1 = decap_t1_active();
		for (const std::string &k : m1_order) {
			const std::vector<int> &qq = by_m1[k];
			for (size_t r = 0; r < en_nr; r++) {
				if (t1) {
					std::unordered_map<std::string, decap_outcome>::const_iterator it =
					    decap_t1_cache.find(decap_t1_key(en_seq[qq[0]].first, en_R[r], n_frames, seat));
					if (it != decap_t1_cache.end() && it->second.ok == 1 && it->second.shadow1 == 0 &&
					    (it->second.lost1 || it->second.lop1 || it->second.over1)) {
						decap_en_out o;
						memset(&o, 0, sizeof(o));
						o.ok = 1;
						o.lost1 = o.lost2 = it->second.lost1 ? 1 : 0;
						o.end1 = 1;
						o.dealt1 = o.dealt2 = it->second.dealt1;
						o.taken1 = o.taken2 = it->second.taken1;
						for (int q : qq) shm[pos[q] * en_nr + r] = o;
						EN.t1_hits++;
						continue;
					}
				}
				grp g;
				g.q0 = qq[0];
				g.r = r;
				g.qs = qq;
				G.push_back(g);
				turns += 1 + (int)qq.size();
			}
		}
		const int W = (int)std::min((size_t)en_disp, std::max((size_t)1, G.size()));
		struct timespec td;
		clock_gettime(CLOCK_MONOTONIC, &td);
		const long left = end - decap_ms_since(t0);
		bool complete = left > 0;
		std::vector<pid_t> kids;
		std::vector<int> kid_w;
		for (int w = 0; w < W && complete && !G.empty(); w++) {
			// [game access]
			if (pid == 0) {
				// [game access: 7 lines]
				int st = 1;
				for (;;) {
					const int gi = __sync_fetch_and_add(qnext, 1);
					if (gi >= (int)G.size()) break;
					if (decap_ms_since(td) >= left) { st = 2; break; }
					const grp &g = G[gi];
					std::vector<const Move *> m2s;
					std::vector<size_t> slots;
					for (int q : g.qs) {
						m2s.push_back(&en_seq[q].second);
						slots.push_back(pos[q] * en_nr + g.r);
					}
					// [game access]
					if (c == 0) {
						// [game access]
						decap_en_group(seat, en_seq[g.q0].first, en_R[g.r], m2s, slots, shm);
						// [game access]
					}
					// [game access]
				}
				__sync_synchronize();
				done[w] = st;
				// [game access]
			}
			if (pid > 0) {
				kids.push_back(pid);
				kid_w.push_back(w);
			}
			else {
				EN.disp_fail++;
				complete = false;
			}
		}
		size_t alive = kids.size();
		while (alive > 0) {
			for (size_t q = 0; q < kids.size(); q++) {
				pid_t &k = kids[q];
				if (k <= 0) continue;
				__sync_synchronize();
				if (done[kid_w[q]]) {
					if (done[kid_w[q]] != 1) complete = false;
					// [game access]
					k = -1;
					alive--;
				}
				// [game access]
					k = -1;
					alive--;
					EN.disp_fail++;
					complete = false;
				}
			}
			if (alive == 0) break;
			if (decap_ms_since(td) > left + 1000) {
				for (pid_t &k : kids) {
					if (k > 0) {
						// [game access: 2 lines]
						k = -1;
						EN.disp_fail++;
					}
				}
				complete = false;
				break;
			}
			usleep(500);
		}
		if (decap_ms_since(t0) > end) complete = false;
		__sync_synchronize();
		int n_val = 0;
		for (size_t i = 0; i < qs.size(); i++) {
			double sum = 0.0;
			bool all = true;
			int na = 0;
			for (size_t r = 0; r < en_nr; r++) {
				const decap_en_out &o = shm[i * en_nr + r];
				double sc;
				if (o.ok != 1) {
					all = false;
					sc = EN_LOSS;
					na++;
				}
				else if (o.lost1) sc = EN_LOSS;
				else if (o.end1) sc = std::max(-EN_CAP, std::min(EN_CAP, (double)o.dealt1 - (double)o.taken1));
				else if (o.lost2) sc = EN_LOSS;
				else sc = std::max(-EN_CAP, std::min(EN_CAP, (double)o.dealt2 - (double)o.taken2));
				sum += sc;
			}
			if (complete || all) {
				en_val[qs[i]] = sum / (double)en_nr;
				EN.n_plan += (int32_t)en_nr;
				EN.noans += na;
				n_val++;
			}
		}
		// [game access]
		*ms_out = decap_ms_since(td);
		*turns_out = turns;
		return complete ? 0 : (n_val > 0 ? 1 : -1);
	};

	// [game access]
	if (!en_rs) en_rs = 0x2545f4914f6cdd1dull;
	auto en_u = [&]() {
		en_rs ^= en_rs >> 12;
		en_rs ^= en_rs << 25;
		en_rs ^= en_rs >> 27;
		return (double)((en_rs * 0x2545f4914f6cdd1dull) >> 11) * (1.0 / 9007199254740992.0);
	};
	double en_p[2][N_JOINTS][4];
	for (int t = 0; t < 2; t++) {
		for (int j = 0; j < N_JOINTS; j++) {
			for (int s = 0; s < 4; s++) en_p[t][j][s] = (1.0 - en_p0) / 3.0;
			const int sn = en_N.states[j];
			if (sn >= 1 && sn <= 4) en_p[t][j][sn - 1] = en_p0;
		}
	}
	auto en_add = [&](const Move &a, const Move &b) {
		const std::string k = en_key(a) + en_key(b);
		std::map<std::string, int>::const_iterator it = en_idx.find(k);
		if (it != en_idx.end()) return it->second;
		en_seq.push_back(std::make_pair(a, b));
		en_val.push_back(NAN);
		en_idx[k] = (int)en_seq.size() - 1;
		return (int)en_seq.size() - 1;
	};
	int en_best = -1;
	double en_bestJ = -1e18;
	const char *en_status = "complete";
	long en_ms_last = 0;
	int en_turns_last = 0;
	int en_nseed = 0;
	for (int g = 0; g < en_gens && en_nr > 0 && !held.empty(); g++) {
		std::vector<int> gen;
		std::set<int> gs;
		if (g == 0) {
			gs.insert(en_add(en_N, en_N));
			gen.push_back(*gs.begin());
			for (size_t k = 0; k < fin.size(); k++) {
				const int q = en_add(cand[fin[k]], cand[fin[k]]);
				if (gs.insert(q).second) gen.push_back(q);
			}
			en_nseed = (int)gen.size();
		}
		for (int i = 0; i < en_pop; i++) {
			Move mv[2] = { en_N, en_N };
			for (int t = 0; t < 2; t++) {
				for (int j = 0; j < N_JOINTS; j++) {
					// [game access]
					const double u = en_u();
					double c = 0.0;
					int s = 0;
					for (; s < 3; s++) {
						c += en_p[t][j][s];
						if (u < c) break;
					}
					mv[t].states[j] = (uint8_t)(s + 1);
				}
			}
			const int q = en_add(mv[0], mv[1]);
			if (gs.insert(q).second) gen.push_back(q);
		}
		std::vector<int> fresh;
		for (int q : gen) if (!std::isfinite(en_val[q])) fresh.push_back(q);

		long ms = 0;
		int turns = 0;
		const int rr = fresh.empty() ? 0 : en_run(fresh, en_cem_end, &ms, &turns);
		EN.ms_gen[g] = (int32_t)ms;
		en_turns += turns;
		if (rr != 0) {
			int fin_n = 0;
			for (int q : gen) {
				if (!std::isfinite(en_val[q])) continue;
				fin_n++;
				if (en_val[q] > en_bestJ) {
					en_bestJ = en_val[q];
					en_best = q;
				}
			}
			EN.partial_seqs = fin_n;
			EN.gen_cut = g + 1;
			if (fin_n > 0) EN.J_gen[g] = (float)en_bestJ;
			// [game access]
			break;
		}
		en_ms_last = ms;
		en_turns_last = turns;
		EN.gens_done = g + 1;

		std::vector<int> pool(gen);
		std::stable_sort(pool.begin(), pool.end(), [&](int a, int b) { return en_val[a] > en_val[b]; });
		if (en_val[pool[0]] > en_bestJ) {
			en_bestJ = en_val[pool[0]];
			en_best = pool[0];
		}
		EN.J_gen[g] = (float)en_bestJ;
		const size_t ne = std::min(pool.size(), (size_t)en_elites);
		double fr[2][N_JOINTS][4];
		memset(fr, 0, sizeof(fr));
		for (size_t e = 0; e < ne; e++) {
			for (int t = 0; t < 2; t++) {
				const Move &m = t == 0 ? en_seq[pool[e]].first : en_seq[pool[e]].second;
				for (int j = 0; j < N_JOINTS; j++) {
					const int s = m.states[j];
					if (s >= 1 && s <= 4) fr[t][j][s - 1] += 1.0 / (double)ne;
				}
			}
		}
		for (int t = 0; t < 2; t++) {
			for (int j = 0; j < N_JOINTS; j++) {
				double sum = 0.0;
				for (int s = 0; s < 4; s++) {
					en_p[t][j][s] = std::max((1.0 - en_mix) * en_p[t][j][s] + en_mix * fr[t][j][s], en_floor);
					sum += en_p[t][j][s];
				}
				for (int s = 0; s < 4; s++) en_p[t][j][s] /= sum;
			}
		}
	}
	// [game access]
	EN.status = (uint8_t)(en_nr == 0 || held.empty() ? 4 : (en_best < 0 ? 3 : (EN.gens_done == en_gens ? 1 : 2)));
	EN.ms_cem = (int32_t)(decap_ms_since(t0) - en_t_start);
	EN.n_seed = en_nseed;
	EN.n_seq = (int32_t)en_seq.size();
	EN.n_turn = en_turns;
	if (EN.status == 2) cem_note |= 524288;
	const char *en_dec = "no entry pick";
	EN.decision = 4;
	if (en_best >= 0) {
		cem_note |= 65536;
		const Move en_F = en_seq[en_best].first;
		EN.J_best = (float)en_bestJ;
		memcpy(EN.F, en_F.states, N_JOINTS);
		memcpy(EN.F2, en_seq[en_best].second.states, N_JOINTS);
		{
			std::map<std::string, int>::const_iterator it = en_idx.find(en_key(en_N) + en_key(en_N));
			if (it != en_idx.end()) EN.J_NN = (float)en_val[it->second];
			it = en_idx.find(en_key(en_F) + en_key(en_N));
			if (it != en_idx.end()) EN.J_F_N2 = (float)en_val[it->second];
			double bn = -1e18;
			for (size_t q = 0; q < en_seq.size(); q++) {
				if (std::isfinite(en_val[q]) && en_key(en_seq[q].first) == en_key(en_N) && en_val[q] > bn) bn = en_val[q];
			}
			if (bn > -1e17) EN.J_Nbest = (float)bn;
		}
		for (size_t c = 0; c < cand.size(); c++) {
			if (en_key(cand[c]) == en_key(en_F)) {
				EN.f_cand = (int32_t)c;
				EN.f_score = c_safe[c] ? stage_score(c) : NAN;
				break;
			}
		}
		if (en_min_gens > 0 && EN.gens_done < en_min_gens) {
			EN.decision = 5;
			// [game access]
			cem_note |= 1048576;
		}
		else if (en_key(en_F) == en_key(en_N)) {
			EN.decision = 0;
			// [game access]
		}
		else {
			const long tc = decap_ms_since(t0);
			const decap_full_t en_sv_full = decap_full;
			const decap_diag_t en_sv_diag = decap_diag;
			const bool en_sv_xp = decap_xp.on;
			decap_xp.on = false;
			std::vector<Move> en_FM(1, en_F);
			memcpy(en_FM[0].grip, en_N.grip, sizeof(en_FM[0].grip));
			const std::vector<Move> *sets[3] = { &theirs, &held, &adv_seen };
			const unsigned seeds[3] = { plan_seed, plan_seed ^ 0x9e3779b9u, plan_seed ^ 0x3c6ef372u };
			bool safe = true, timed_out = false;
			for (int s = 0; s < 3; s++) {
				EN.n_set[s] = (uint8_t)std::min((size_t)255, sets[s]->size());
				if (sets[s]->empty()) continue;
				const long left = en_safe_end - decap_ms_since(t0);
				if (left <= 0) {
					timed_out = true;
					safe = false;
					EN.why[s] = 4;
					EN.fail_sets |= (uint8_t)(1 << s);
					continue;
				}
				std::vector<decap_outcome> tmp;
				decap_plan_budget_ms = left;
				decap_plan_outcomes = &tmp;
				decap_lookahead(seat, en_FM, *sets[s], n_frames, skill_level, seeds[s]);
				decap_plan_outcomes = nullptr;
				decap_plan_budget_ms = -1;
				int na = 0;
				for (const decap_outcome &o : tmp) na += o.ok == 1;
				EN.n_ans[s] = (uint8_t)std::min(na, 255);
				const bool ok = decap_full.ok(0) && na == (int)sets[s]->size();
				EN.why[s] = decap_full.why.empty() ? 4 : decap_full.why[0];
				if (!ok) {
					safe = false;
					EN.fail_sets |= (uint8_t)(1 << s);
					if (na < (int)sets[s]->size() && decap_ms_since(t0) >= en_safe_end) timed_out = true;
				}
			}
			decap_full = en_sv_full;
			decap_diag = en_sv_diag;
			decap_xp.on = en_sv_xp;
			EN.ms_safe = (int32_t)(decap_ms_since(t0) - tc);
			if (safe) {
				EN.decision = 1;
				en_dec = "OVERRIDE";
				ko_changed = true;
				cem_note |= 131072;
				chosen = en_FM[0];
				if (EN.f_cand >= 0) played_c = EN.f_cand;
				else {
					cand.push_back(en_FM[0]);
					c_safe.push_back(1);
					c_mean.push_back(NAN);
					c_min.push_back(NAN);
					played_c = (int)cand.size() - 1;
				}
				cem_note &= ~1;
				if ((size_t)played_c >= mine.size() && EN.f_cand >= 0 && !lp_is((size_t)played_c)) cem_note |= 1;
			}
			else {
				EN.decision = timed_out ? 3 : 2;
				// [game access]
				cem_note |= 262144;
			}
		}
	}
	EN.ms_stage = (int32_t)(decap_ms_since(t0) - en_t_start);
	if (en_log) {
		char jn[N_JOINTS + 1], jf[N_JOINTS + 1], j2[N_JOINTS + 1];
		for (int j = 0; j < N_JOINTS; j++) {
			jn[j] = (char)('0' + EN.N[j]);
			jf[j] = (char)('0' + (en_best >= 0 ? EN.F[j] : 0));
			j2[j] = (char)('0' + (en_best >= 0 ? EN.F2[j] : 0));
		}
		jn[N_JOINTS] = jf[N_JOINTS] = j2[N_JOINTS] = 0;
		char line[1200];
		snprintf(line, sizeof(line),
		         // [game access: 7 lines]
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
