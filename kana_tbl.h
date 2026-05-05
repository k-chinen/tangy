/*
 * kana_tbl.h
 *
 * UTF-8 encoded Japanese kana character tables.
 * This file is intentionally separated from other source files
 * to confine multi-byte (UTF-8) characters to one place,
 * avoiding encoding issues in tools such as grep and diff.
 *
 * Encoding: UTF-8
 */

#ifndef _TANGY_KANA_TBL_H_
#define _TANGY_KANA_TBL_H_

/*
 * Hiragana in gojuuon order (五十音順・ひらがな)
 * あいうえお かきくけこ さしすせそ たちつてと なにぬねの
 * はひふへほ まみむめも やゆよ らりるれろ わをん
 * 46 characters
 */
static const char * const hiragana_gojuuon_tbl[] = {
    "あ","い","う","え","お",   /* a  i  u  e  o  */
    "か","き","く","け","こ",   /* ka ki ku ke ko */
    "さ","し","す","せ","そ",   /* sa si su se so */
    "た","ち","つ","て","と",   /* ta ti tu te to */
    "な","に","ぬ","ね","の",   /* na ni nu ne no */
    "は","ひ","ふ","へ","ほ",   /* ha hi hu he ho */
    "ま","み","む","め","も",   /* ma mi mu me mo */
    "や","ゆ","よ",             /* ya    yu    yo */
    "ら","り","る","れ","ろ",   /* ra ri ru re ro */
    "わ","を","ん",             /* wa wo n        */
};
#define HIRAGANA_GOJUUON_NUM \
    (int)(sizeof(hiragana_gojuuon_tbl)/sizeof(hiragana_gojuuon_tbl[0]))

/*
 * Katakana in gojuuon order (五十音順・カタカナ)
 * アイウエオ カキクケコ サシスセソ タチツテト ナニヌネノ
 * ハヒフヘホ マミムメモ ヤユヨ ラリルレロ ワヲン
 * 46 characters
 */
static const char * const katakana_gojuuon_tbl[] = {
    "ア","イ","ウ","エ","オ",   /* a  i  u  e  o  */
    "カ","キ","ク","ケ","コ",   /* ka ki ku ke ko */
    "サ","シ","ス","セ","ソ",   /* sa si su se so */
    "タ","チ","ツ","テ","ト",   /* ta ti tu te to */
    "ナ","ニ","ヌ","ネ","ノ",   /* na ni nu ne no */
    "ハ","ヒ","フ","ヘ","ホ",   /* ha hi hu he ho */
    "マ","ミ","ム","メ","モ",   /* ma mi mu me mo */
    "ヤ","ユ","ヨ",             /* ya    yu    yo */
    "ラ","リ","ル","レ","ロ",   /* ra ri ru re ro */
    "ワ","ヲ","ン",             /* wa wo n        */
};
#define KATAKANA_GOJUUON_NUM \
    (int)(sizeof(katakana_gojuuon_tbl)/sizeof(katakana_gojuuon_tbl[0]))

/*
 * Hiragana in iroha order (伊呂波順・ひらがな)
 * いろはにほへと ちりぬるを わかよたれそつね
 * ならむ うゐのおく やまけふこ えてあさきゆめみし ゑひもせす
 * 47 characters (including archaic ゐ wi and ゑ we)
 */
static const char * const hiragana_iroha_tbl[] = {
    "い","ろ","は","に","ほ","へ","と",   /* i  ro ha ni ho he to */
    "ち","り","ぬ","る","を",             /* ti ri nu ru wo       */
    "わ","か","よ","た","れ","そ","つ","ね", /* wa ka yo ta re so tu ne */
    "な","ら","む",                       /* na ra mu             */
    "う","ゐ","の","お","く",             /* u  wi no o  ku       */
    "や","ま","け","ふ","こ",             /* ya ma ke hu ko       */
    "え","て","あ","さ","き","ゆ","め","み","し", /* e te a sa ki yu me mi si */
    "ゑ","ひ","も","せ","す",             /* we hi mo se su       */
};
#define HIRAGANA_IROHA_NUM \
    (int)(sizeof(hiragana_iroha_tbl)/sizeof(hiragana_iroha_tbl[0]))

/*
 * Katakana in iroha order (伊呂波順・カタカナ)
 * イロハニホヘト チリヌルヲ ワカヨタレソツネ
 * ナラム ウヰノオク ヤマケフコ エテアサキユメミシ ヱヒモセス
 * 47 characters (including archaic ヰ wi and ヱ we)
 */
static const char * const katakana_iroha_tbl[] = {
    "イ","ロ","ハ","ニ","ホ","ヘ","ト",   /* i  ro ha ni ho he to */
    "チ","リ","ヌ","ル","ヲ",             /* ti ri nu ru wo       */
    "ワ","カ","ヨ","タ","レ","ソ","ツ","ネ", /* wa ka yo ta re so tu ne */
    "ナ","ラ","ム",                       /* na ra mu             */
    "ウ","ヰ","ノ","オ","ク",             /* u  wi no o  ku       */
    "ヤ","マ","ケ","フ","コ",             /* ya ma ke hu ko       */
    "エ","テ","ア","サ","キ","ユ","メ","ミ","シ", /* e te a sa ki yu me mi si */
    "ヱ","ヒ","モ","セ","ス",             /* we hi mo se su       */
};
#define KATAKANA_IROHA_NUM \
    (int)(sizeof(katakana_iroha_tbl)/sizeof(katakana_iroha_tbl[0]))

#endif/*_TANGY_KANA_TBL_H_*/
