CC       = gcc
CFLAGS   = -Wall -Wextra -g -Iinc -pthread
LDFLAGS  = -pthread -lsqlite3

SRC_DIR    = src
OBJ_DIR    = obj
INC_DIR    = inc
LIB_STATIC_DIR = lib/static
LIB_SHARE_DIR  = lib/share
LOG_DIR    = log

# --- Module dùng chung cho thư viện sbuffer (tách riêng để build .a / .so) ---
SBUFFER_SRC = $(SRC_DIR)/sbuffer.c

# --- Toàn bộ module của server (gateway) ---
SERVER_SRCS = $(SRC_DIR)/main.c \
              $(SRC_DIR)/sbuffer.c \
              $(SRC_DIR)/connection_mgr.c \
              $(SRC_DIR)/data_mgr.c \
              $(SRC_DIR)/storage_mgr.c \
              $(SRC_DIR)/logger.c
SERVER_OBJS = $(SERVER_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all clean dirs static shared

all: dirs server sensor_node

# --- Tạo sẵn các thư mục cần thiết nếu chưa có ---
dirs:
	mkdir -p $(OBJ_DIR) $(LIB_STATIC_DIR) $(LIB_SHARE_DIR) $(LOG_DIR)

# ================= SERVER (Gateway) =================
server: dirs $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJS) $(LDFLAGS)

# ================= SENSOR_NODE (client giả lập) =================
sensor_node: dirs $(OBJ_DIR)/sensor_node.o $(OBJ_DIR)/sbuffer.o
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)/sensor_node.o $(OBJ_DIR)/sbuffer.o $(LDFLAGS)

# ================= Quy tắc chung: biên dịch .c -> .o =================
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ================= Static library (ar) =================
# Đóng gói sbuffer.o thành libsbuffer.a để nộp kèm minh chứng hiểu về static lib
static: dirs $(OBJ_DIR)/sbuffer.o
	ar rcs $(LIB_STATIC_DIR)/libsbuffer.a $(OBJ_DIR)/sbuffer.o
	@echo "Created $(LIB_STATIC_DIR)/libsbuffer.a"

# ================= Shared library (.so) =================
# Cần biên dịch lại với -fPIC (Position Independent Code) riêng, không dùng chung .o với static
shared: dirs
	$(CC) $(CFLAGS) -fPIC -c $(SBUFFER_SRC) -o $(OBJ_DIR)/sbuffer_pic.o
	$(CC) -shared -o $(LIB_SHARE_DIR)/libsbuffer.so $(OBJ_DIR)/sbuffer_pic.o
	@echo "Created $(LIB_SHARE_DIR)/libsbuffer.so"

# ================= Dọn dẹp =================
clean:
	rm -rf $(OBJ_DIR)/*.o server sensor_node
	rm -f $(LIB_STATIC_DIR)/*.a $(LIB_SHARE_DIR)/*.so
	rm -f gateway.log sensor_data.db logFifo
	rm -f $(LOG_DIR)/*.log