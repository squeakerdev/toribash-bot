# decapAI: a search bot for Toribash

decapAI is a bot that plays the physics fighting game Toribash in one mod (lenshu3ng). Each turn it picks a move by
simulating many moves and keeping the one that scores best against the replies it expects.

This repository shows the bot's decision logic at build B53. It is here to be read, not run.

## What is here and what is not

The bot runs inside a private build of the game. This repository shows the bot's decision logic only, with everything
that touches the game taken out.

| Shown | Left out |
|---|---|
| The bot's special-stage files (`bot/`) | All game and server source |
| The bot's central search (`bot/core/`) | Every statement of the bot's that reads or calls the game, and every function that mainly does |
| Three test tools (`tools/`) | All original comments: they describe how the game works inside |
| | All physics and simulation code, and the planners that run on it |
| | The code that connects the bot to the game, the network evaluation code, the test tools that run positions through the bot |
| | Model files, game records, the room's scripts and settings |

Markers show where something was taken out:

- `// [game access]` or `// [game access: N lines]` stands for statements that read or call the game, or that manage
  the processes the search runs in.
- `[REDACTED: ...]` stands for a function body or a block.
- The game's own type, constant and variable names are replaced by neutral ones, such as `Move`, `Position`,
  `N_JOINTS` and `STATE_HOLD`.
- `<notes>` and `<work>` replace folder paths on the author's machine.

Because of these cuts nothing here compiles or runs, and the files have no comments beyond a short header. The
sections below explain what the code does.

## How a move is chosen

The main flow is `decap_choose_move` in `bot/core/search_core.cpp`. In order:

1. **Read the position.** Both fighters' body positions, speeds, joint states, damage and score at the start of the turn.
2. **Collect candidate moves.** A move is one state (extend, contract, hold, relax) for each of the 20 joints. Candidates
   come from a policy network's draws, "keep my joints", moves that strong players played from similar positions (a
   nearest-neighbour index), hand-written edits and jump templates. Duplicates are removed.
3. **Guess the opponent's replies.** The opponent's move for the turn is hidden from the bot, as it is from a human, so
   the bot works with guesses: he keeps his joints, network draws for his side, and what strong players did from his
   position.
4. **Play each candidate against each guess.** Every pair is simulated for the candidate turn and two or three likely
   turns after it. The end position is scored by a value network plus points, dismemberments and ring-out risk.
5. **Drop unsafe moves.** A move that gets the bot disqualified, or leaves it close to a disqualification, is removed.
6. **Test the finalists harder.** The best few are replayed against held-out replies, adversarial replies, and a counter
   search that looks for the opponent's best answer to each one.
7. **Pick.** A weighted draw among the safe finalists, on a mix of each one's average and worst score.

Stages that take over in particular situations, each behind a switch (not all are on in the live room):

| File | Stage | When |
|---|---|---|
| `decap_open.inc` | Opening search: every book opener against the opponent's likely first moves, then a second-turn tournament | Turn 0 |
| `decap_race.inc` | Race: a wide pool of one-turn moves cut down in rounds (512, 128, 32, 8) as replies are added | Round one of the search |
| `decap_ko.inc` | Knockout: a large pool scored on one-turn points, may override the pick | Close range |
| `decap_entry.inc`, `decap_entry_sim.inc` | Entry planner: a search over two-move sequences | 2.5 to 3.5 m apart |
| `decap_cl_play.inc` | Chase lab: when its first move is played in place of the bot's own pick | Far apart |
| `decap_t2_play.inc` | Chase search (new in B53): closing sequences tested against 7 guessed replies, scored half average and half worst case; this file decides when it runs and when its move is played | 2.3 m or more apart, after the first turn |
| `decap_speed.inc` | A cache of first-turn results | Speed only |
| `decap_logfull.inc` | The full decision log: every candidate, reply, score and stage of a decision | Logging only |

The search code of the two chase stages, and a third planner that plans against the opponent's likely paths, are not
published.

The search starts when the turn starts and keeps its best move so far. That move is played when the search finishes
or when the turn ends.

## What the bot learns from

- Replays of strong players in this mod: they train the networks and fill the nearest-neighbour index.
- Its own games: every decision is logged and used to test changes.
- In the opening only, the first and second moves an opponent has played against it before. When the bot has met a
  player, it weights that player's past openers above the average player's.

No game records, player names or model files are in this repository.

## How changes are tested (`tools/`)

| Tool | What it does |
|---|---|
| `holdout.py` | Honest numbers: games are split once, by a hash, into "explore" (60%) and "final" (40%). Ideas are tuned on explore positions only. A built version gets one run on final positions it has never met, with the pass rule written to a file first. Only that number is quoted |
| `compare.py` | Identity check: compares the decision records of two builds field by field. A new build with its new switches off must decide exactly as the live build on 31 frozen real positions |
| `dec_read.py` | Reader for the bot's decision logs |

The tools that run positions through the bot are not included.

## Build B53 result

The chase search was compared with the same build with the switch off, on 150 positions from 118 games that neither
had seen:

| Measure | Chase search on | Off | Difference (95% range) |
|---|---|---|---|
| Ground closed in two turns | 1.03 m | 0.46 m | +0.57 m (0.43 to 0.71) |
| Points over three turns | +5,622 | +3,873 | +1,749 (+91 to +3,341) |
| Points after one turn | | | -422 (-1,055 to +117) |

This is a test on recorded games: the recorded human could not react to the new moves. Real games are the test that
counts.

## Licence

Shown for reading; no licence to use it is granted. Toribash belongs to its owners, and none of its source is in this
repository.
