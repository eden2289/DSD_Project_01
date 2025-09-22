# --------------------
# Makefile for ROBDD Project (修改版：執行檔輸出至根目錄)
# --------------------

# 1. Compiler and Flags
CXX = g++
CXXFLAGS = -std=c++11 -O3

# 2. Project Structure
# 目標執行檔名稱
TARGET_EXEC = robdd

# 來源檔案 (修正：自動尋找 src/ 底下所有 .cpp 檔案)
SRCS = $(wildcard src/*.cpp)
DOT_DIR = dot
PNG_DIR = png

# 3. Rules

# 預設規則：輸入 "make" 或 "make all" 就會執行這裡
all: $(TARGET_EXEC)

# 編譯規則：告訴make如何從.cpp檔案生成執行檔
$(TARGET_EXEC): $(SRCS)
	@echo "Compiling $(TARGET_EXEC)..."
	@$(CXX) $(CXXFLAGS) -o $(TARGET_EXEC) $(SRCS)
	@echo "Compilation finished."

# 執行規則：可以用 "make run" 來編譯並執行
run: $(TARGET_EXEC)
	@./$(TARGET_EXEC) $(ARGS)

# 測試規則：用 "make test" 執行預設的輸入檔案
test: $(TARGET_EXEC)
	@mkdir -p $(DOT_DIR)
	@./$(TARGET_EXEC) pla_files/input.pla $(DOT_DIR)/output.dot

# 清理規則：用 "make clean" 來刪除編譯好的檔案和所有產出的圖檔
clean:
	@echo "Cleaning executable and all generated files in dot/ and png/..."
	@rm -f $(TARGET_EXEC)
	@rm -rf $(DOT_DIR) $(PNG_DIR)

.PHONY: all run clean test run4 run5 png package clean_package

# --- 動態 PLA 處理規則 ---
# 讓 'make my4' 或 'make pla_files/my4.pla' 都能觸發規則
# % 會匹配 'my4' 或 'pla_files/my4.pla'
# 範例: make my4
#       make pla_files/another.pla
.PHONY: %
%:
	@# 檢查對應的 .pla 檔案是否存在於 pla_files/ 中
	@if [ -f "pla_files/$(basename $(notdir $@)).pla" ]; then \
		PLA_NAME=$(basename $(notdir $@)); \
		echo "==> Processing pla_files/$${PLA_NAME}.pla"; \
		mkdir -p $(DOT_DIR); \
		TEMP_DOT_FILE="$(DOT_DIR)/$${PLA_NAME}_temp.dot"; \
		./$(TARGET_EXEC) "pla_files/$${PLA_NAME}.pla" "$${TEMP_DOT_FILE}"; \
		\
		ROBDD_TEMP_FILE="$${TEMP_DOT_FILE}"; \
		OBDD_TEMP_FILE="$(DOT_DIR)/$${PLA_NAME}_temp_obdd.dot"; \
		\
		ROBDD_FINAL_FILE="$(DOT_DIR)/$${PLA_NAME}_robdd.dot"; \
		OBDD_FINAL_FILE="$(DOT_DIR)/$${PLA_NAME}_obdd.dot"; \
		\
		echo "  -> Renaming $$OBDD_TEMP_FILE to $$OBDD_FINAL_FILE"; \
		mv "$$OBDD_TEMP_FILE" "$$OBDD_FINAL_FILE"; \
		\
		echo "  -> Renaming $$ROBDD_TEMP_FILE to $$ROBDD_FINAL_FILE"; \
		mv "$$ROBDD_TEMP_FILE" "$$ROBDD_FINAL_FILE"; \
	else \
		echo "make: *** No rule to make target '$@'. Stop." >&2; \
		exit 1; \
	fi

# --- 批次轉檔規則 ---
# 轉出 PNG（需安裝 Graphviz）
# 會自動尋找根目錄與 pla_files/ 下的所有 .dot 檔案並轉換
png:
	@# 檢查 'dot' 指令是否存在
	@which dot >/dev/null 2>&1 || { echo "Graphviz not found. On macOS, run: brew install graphviz"; exit 1; }
	@echo "Converting all .dot files from $(DOT_DIR)/ to .png files in $(PNG_DIR)/..."
	@mkdir -p $(PNG_DIR)
	@# 遍歷所有在 dot/ 資料夾中的 .dot 檔案
	@for dotfile in $(wildcard $(DOT_DIR)/*.dot); do \
		if [ -f "$$dotfile" ]; then \
			filename=$$(basename "$$dotfile"); \
			pngfile="$(PNG_DIR)/$${filename%.dot}.png"; \
			echo "  -> Converting $$dotfile to $$pngfile"; \
			dot -Tpng "$$dotfile" -o "$$pngfile"; \
		fi \
	done
	@echo "Conversion complete."

# --- 打包提交 ---
# 用法: make package SID=你的學號 (例如: make package SID=B12345678)
package:
	@# 檢查是否提供學號
	@if [ -z "$(SID)" ]; then \
		echo "Usage: make package SID=<Your_Student_ID>"; \
		exit 1; \
	fi
	@echo "Creating package $(SID).tar.gz..."
	@tar -czvf $(SID).tar.gz src/main.cpp Makefile
	@echo "Package created successfully."

# 清理打包產生的檔案
clean_package:
	@echo "Cleaning up package files..."
	@rm -f $(SID).tar.gz