# ROBDD Generation (DSD Project 1)

This program reads a single-output Boolean function in PLA format, builds an OBDD with a fixed variable order, reduces it to an ROBDD, and outputs DOT files for visualization.

Quick run
- Build: `make`
- Example: `./robdd pla_files/input.pla output.dot`
- PNGs (requires Graphviz):
  - `dot -Tpng output_obdd.dot -o obdd_graph.png`
  - `dot -Tpng output.dot -o graph.png`

## Build and Run
- Dependencies: g++ (C++17), make. For visualization: Graphviz (`sudo apt-get install graphviz`).
- Build: `make`
- Run: `./robdd <PLA_FILE> <DOT_FILE>`
  - Writes OBDD as `<DOT_FILE>` with `_obdd` suffix
  - Writes ROBDD as `<DOT_FILE>`

Example
```
make
./robdd pla_files/input.pla output.dot
# visualize
dot -Tpng output_obdd.dot -o obdd_graph.png
dot -Tpng output.dot -o graph.png
```

## Supported PLA format
The parser supports a simplified PLA:
- `.i N` number of inputs
- `.ilb <v1> <v2> ... <vN>` variable names (order = decision order)
- `.p K` number of product terms (optional hint; used to reserve capacity)
- `.e` end of file
- Product terms: `<cube> <out>` where `<out>` must be `1` to include the term in the on-set
- Other directives like `.o`, `.ob`, `.type` are ignored

Example (`pla_files/input.pla`)
```
.i 3
.o 1
.ilb a b c
.p 2
11- 1
--1 1
.e
```

## Execution Flow
1) Parse PLA (`PlaParser`)
- Reads `num_inputs`, `var_names`, and keeps only on-set product terms (`output == "1"`).

2) Build OBDD (`RobddManager::buildObddFromPla`)
- Recursively descends variables in the listed order (`.ilb`).
- For each level, splits terms into else/then sets by current literal (`0`, `1`, `-`).
- Creates a full binary decision tree without reduction using `makeNodeNoReduce`, assigning each internal node a stable `obdd_index` (heap index: root=1, else=2*i, then=2*i+1).
- Terminals: 0 and 1 at fixed IDs.

3) Reduce to ROBDD (`RobddManager::reduceToRobdd`)
- Bottom-up via `reduceRec` with memoization:
  - Rule 1 (redundant test): if children equal, return child.
  - Rule 2 (merge isomorphic): unique table on `(var_index, else_id, then_id)`.
- Produces a compact `nodes` vector; preserves each node's original `obdd_index` for readable DOT ranks.

4) Generate DOT files
- `writeObddDot(...)`: full OBDD with ranks grouped by `obdd_index` levels.
- `writeRobddDot(...)`: ROBDD nodes labeled and ranked by their preserved `obdd_index`; edges point to terminal 0 or terminal 1 index and to internal node indices.

## OBDD vs ROBDD in the code
- OBDD: built once without on-the-fly reduction. All internal nodes have `obdd_index`.
- ROBDD: reduced from the built OBDD. Nodes keep their original `obdd_index` (if they came from OBDD) so the drawing is level-aligned with the variable order.

## Variable Ordering
- The decision order is exactly the order in `.ilb` (e.g., `a b c`). Changing this order changes both the OBDD size and the resulting ROBDD.

## Algorithmic Notes
- OBDD build: time roughly O(K * N) where K is number of product terms and N is inputs, given simple partitioning at each level.
- Reduction: time roughly O(M log M) or O(M) average with hashing, where M is number of OBDD nodes; relies on unique-table hashing of `(var, else, then)`.

## Sample PLA files for the report
Two additional PLA examples are provided as required by the assignment.

### 4-variable example: f(a,b,c,d) = (a & b) | (c & d)
`pla_files/my4.pla`
```
.i 4
.o 1
.ilb a b c d
.p 2
11-- 1
--11 1
.e
```

### 5-variable example: f(a,b,c,d,e) = (a & b) | (c & d) | e
`pla_files/my5.pla`
```
.i 5
.o 1
.ilb a b c d e
.p 3
11--- 1
--11- 1
----1 1
.e
```

You can generate DOT/PNGs:
```
./robdd pla_files/my4.pla my4.dot
./robdd pla_files/my5.pla my5.dot

dot -Tpng my4_obdd.dot -o my4_obdd.png
dot -Tpng my4.dot -o my4_robdd.png

dot -Tpng my5_obdd.dot -o my5_obdd.png
dot -Tpng my5.dot -o my5_robdd.png
```

## Report Guidance (what to include)
- Problem statement: What BDD/OBDD/ROBDD are; reduction rules.
- Variable ordering used and rationale.
- Your two Boolean functions (4, 5 vars) and their PLA listings.
- The generated OBDD and ROBDD DOT contents (or screenshots of PNGs).
- Brief algorithm description and complexity.
- Observations: OBDD size vs ROBDD size for your examples.
- How to build/run (copy from this README).

## Packaging for submission
Create a tarball containing source, your PLA files, generated DOTs/PNGs, and a short README/report PDF as required:
```
# from repo root
mkdir -p submit
cp -r src Makefile pla_files *.dot *.png README.md submit/ 2>/dev/null || true
# add your PDF report and any extra PLA/DOT/PNG files
# e.g., cp report.pdf submit/
tar czf B12345678.tgz -C submit .
```

## Repository Structure
- `src/`: sources (`PlaParser`, `RobddManager`)
- `pla_files/`: sample PLA files
- `robdd`: compiled binary after `make`
- `output_obdd.dot`, `output.dot`: default outputs from running the example

## 中文簡述
- 先用 PLA 建 OBDD（固定變數順序），再依兩個規則化簡成 ROBDD：
  1) 若 then/else 指到同一個子圖，刪除該測試。
  2) 具有相同 `(變數, else, then)` 的節點合併。
- `output_obdd.dot` 與 `output.dot` 分別為 OBDD/ROBDD 圖；用 Graphviz 轉成 PNG 截圖即可附在報告中。
