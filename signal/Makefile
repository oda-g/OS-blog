alarm: alarm.c
	zig cc -target riscv64-linux-musl alarm.c -o alarm

count_ng: count_ng.c
	gcc -o count_ng count_ng.c

count_ok: count_ok.c
	gcc -o count_ok count_ok.c -pthread
