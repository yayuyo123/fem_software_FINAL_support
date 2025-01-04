# 解析ソフトFINALの補助ツール

## 作りたい機能
- rcsモデリング
- 出力ファイルの整形
- cliでテキストディタ

## 要素タイプ番号
- 六面体要素
  
| TYPH | 部分 |
|---|---|
| 1 | 柱コンクリート |
| 2 | 柱かぶりコンクリート |
| 3 | 柱梁接合部内部要素 |
| 4 | 柱梁接合部外部要素 |
| 5 | 加力治具 |

- 四辺形要素(仮)

| TYPQ | 部分 |
|---|---|
| 1 | 梁ウェブ |
| 2 | 接合部ウェブ |
| 3 | 梁フランジ上 |
| 4 | 梁フランジ下 |
| 5 | 直交梁梁ウェブ |
| 6 | 直交梁フランジ上 |
| 7 | 直交梁フランジ下 |
| 8 | ふさぎ板1 |
| 9 | ふさぎ板2 |
| 10 | ふさぎ板3 |

- フィルム要素

| TYPF | 部分 |
|---|---|
| 1 |x軸直交面ふさぎ板|
| 2 |x軸直交面ウェブ|
| 3 |y軸直交面ふさぎ板|
| 4 |z軸直交面ふさぎ板|
