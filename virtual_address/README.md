# 仮想空間補足

## ビルド、実行

[setjump](https://github.com/oda-g/OS-blog/tree/main/setjmp)同様、RISC-V用バイナリを作成、実行。

```
$ zig cc -target riscv64-linux-musl addr.c -o addr
$ qemu-riscv64 addr
```

## アセンブルコード

llvm-objdumpで、コードの15行目「*((int *)0x10280a8) = 10;」がどうなっているのか見ると、以下のようになっている。

```
 10087c6: b7 85 02 01   lui     a1, 4136        a1 = 4136
 10087ca: 29 45         li      a0, 10          a0 = 10
 10087cc: 23 a4 a5 0a   sw      a0, 168(a1)     *(a1 << 12 + 168) = a0
```

C風に書くと以下のとおり。
```
a1 = 4136
a0 = 10
*(a1 << 12 + 168) = a0  ==>  *(4136 << 12 + 168) = 10  ==> 4136 << 12 + 168 == 16941224 == 0x10280a8
```

確かに0x10280a8番地に10をストアしている。

## RISC-Vの仮想空間(メモ)


