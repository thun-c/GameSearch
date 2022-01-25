# GameSearch

## 概要
[世界四連覇AIエンジニアがゼロから教えるゲーム木探索入門](https://qiita.com/thun-c/items/058743a25c37c87b8aa4)のサンプルコード。  
それぞれのcppファイルで独立に実行ファイルをコンパイルして動作する。  
main関数内でplayGameを呼ぶと、引数に与えたAIでゲームを開始し、ゲーム状況を標準出力する。  
OnePlayerGameのmain関数内でtestAiScoreを呼ぶと、引数に与えたAIでゲームを複数回行い、平均スコアを表示する。  
AlternateGame,SimultaneousGame のmain関数内でtestFirstPlayerWinRateを呼ぶと、複数回ゲームを行い、引数に与えたAI2つのうち、0番目のAIの勝率を表示する。  

## サンプルコードの特徴
一人ゲーム、交互着手二人ゲーム、同時着手二人ゲーム、それぞれ1つずつサンプルのゲームが実装されている。  
実装済みのサンプルStateクラスの[どのゲームでも実装する]印付きメソッドを実装したクラスを実装すれば、他のゲームでもコードを変更せずにそのままサンプルの探索アルゴリズムを適用して実行可能。  

## メソッドの命名規則
- アルゴリズム名Action : メソッド名に記載されたアルゴリズムで次の行動を決定し、戻り値として返す。  
- アルゴリズム名ActionWithTimeThreshold : メソッド名に記載されたアルゴリズムで、引数で与えられた時間制限内で次の行動を決定し、戻り値として返す。  

## フォルダ構成

```
├── build_script
│   └── build_all.sh         # ソースコードをコンパイルして3つの実行ファイルを作成する。
├── source                   # サンプルコード
│   ├── OnePlayerGame.cpp    # 一人ゲームのサンプルコード
│   ├── AlternateGame.cpp    # 交互着手二人ゲームのサンプルコード
│   └── SimultaneousGame.cpp # 同時着手二人ゲームのサンプルコード
```

## ビルド方法

以下のコマンドを実行する
```
cd build_script
./build_all.sh
```