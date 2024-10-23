count_ng: count_ng.c
	gcc -o count_ng count_ng.c

count_ok: count_ok.c
	gcc -o count_ok count_ok.c -pthread

count_at: count_at.c
	gcc -o count_at count_at.c

rv_count_ng: count_ng.c
	zig cc -target riscv64-linux-musl count_ng.c -o rv_count_ng

rv_count_ok: count_ok.c
	zig cc -target riscv64-linux-musl count_ok.c -o rv_count_ok

rv_count_at: count_at.c
	zig cc -target riscv64-linux-musl count_at.c -o rv_count_at
