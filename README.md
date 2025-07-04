# OS-blog

[VA Linux エンジニアブログ](https://valinux.hatenablog.com/about) のブログ「OS徒然草」の補足的資料です。

補足:  
記事から参照されている資料は、原則、変更はしないつもりです(誤記修正、補足追記の可能性あり)。

## ブログ「OS徒然草」一覧

- [OS徒然草(1)](https://valinux.hatenablog.com/entry/20240725)  
本記事からの参照はありません。
- [OS徒然草(2)](https://valinux.hatenablog.com/entry/20240827)  
  - [setjumpコード、補足](https://github.com/oda-g/OS-blog/tree/main/setjmp)
  - [コンテキストスイッチコード、補足](https://github.com/oda-g/OS-blog/tree/main/context_switch)
- [OS徒然草(3)](https://valinux.hatenablog.com/entry/20240919)
  - [仮想空間メモ](https://github.com/oda-g/OS-blog/tree/main/virtual_address)
- [OS徒然草(4)](https://valinux.hatenablog.com/entry/20241024)  
本記事からの参照はありません。
- [OS徒然草(5)](https://valinux.hatenablog.com/entry/20241121)
  - [競合状態補足](https://github.com/oda-g/OS-blog/tree/main/race_condition)
  - [シグナル処理補足](https://github.com/oda-g/OS-blog/tree/main/signal)
- [OS徒然草(6)](https://valinux.hatenablog.com/entry/20241226)
  - [ファイル管理補足](https://github.com/oda-g/OS-blog/tree/main/file_management)
- [OS徒然草(7)](https://valinux.hatenablog.com/entry/20250130)  
本記事からの参照はありません。
- [OS徒然草(8)](https://valinux.hatenablog.com/entry/20250424)  
本記事からの参照はありません。
- [OS徒然草(9)](https://www.valinux.co.jp/blog/entry/20250522)
  - [マルチスレッド](https://github.com/oda-g/OS-blog/tree/main/multi_thread)
- [OS徒然草(10)](https://www.valinux.co.jp/blog/entry/20250626)  
本記事からの参照はありません。

OS徒然草(10)で一旦完結です。

## 補足

### マシン環境

本リポジトリに格納しているプログラムは、x86_64、ubuntu22.04 の環境で動作しています。

```
$ uname -a
Linux study 5.15.0-112-generic #122-Ubuntu SMP Thu May 23 07:48:21 UTC 2024 x86_64 x86_64 x86_64 GNU/Linux
```

### RISC-V仕様書

「OS徒然草(1)」で記述してますが、本リポジトリでも良く参照しているので、ここにもURLを記載しておきます。

https://riscv.org/technical/specifications/

- Volume 1, Unprivileged Specification version 20191213  
https://github.com/riscv/riscv-isa-manual/releases/tag/Ratified-IMAFDQC
- Volume 2, Privileged Specification version 20211203  
https://github.com/riscv/riscv-isa-manual/releases/tag/Priv-v1.12

それぞれ、「仕様書(一般編)」「仕様書(特権編)」と言う名前で参照しています。
