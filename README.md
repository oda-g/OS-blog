# OS-blog

[VA Linux エンジニアブログ](https://valinux.hatenablog.com/about) のブログ「OS徒然草」の補足的資料です。

注意:  
社内レビューの関係上、下書きの時点で公開しているので、予告なく変更されることがあります。
公開済の記事から参照されているものは、原則、変更はしないつもりです(誤記修正、補足追記の可能性あり)。

## ブログ「OS徒然草」一覧

- [OS徒然草(1)](https://valinux.hatenablog.com/entry/20240725)  
本記事からの参照はありません。
- OS徒然草(2) (レビュー中)(8/27公開予定)  
  - [setjumpコード、補足](https://github.com/oda-g/OS-blog/tree/main/setjmp)
  - [コンテキストスイッチコード、補足](https://github.com/oda-g/OS-blog/tree/main/context_switch)
- OS徒然草(3) (執筆中)

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
