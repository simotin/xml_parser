#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "xml.h"

/* XML 解析状態 */
enum XML_PARSE_STATUS {
    XML_PARSE_STATUS_WAIT_STAG_OPEN,                                /* 開始タグ < 待ち                  */
    XML_PARSE_STATUS_WAIT_STAG_CLOSE,                               /* 開始タグ > 待ち                  */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_SSTR,                           /* 開始タグ 属性 開始文字 待ち      */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUAL,                          /* 開始タグ 属性 = 待ち             */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_SQUOTE,                         /* 開始タグ 属性 開始引用符 待ち    */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUOTE,                         /* 開始タグ 属性 終了引用符 待ち    */
    XML_PARSE_STATUS_WAIT_ETAG_OPEN,                                /* 終了タグ < 待ち                  */
    XML_PARSE_STATUS_WAIT_ETAG_SLASH,                               /* 終了タグ / 待ち                  */
    XML_PARSE_STATUS_WAIT_ETAG_CLOSE                                /* 終了タグ > 待ち                  */
};

int32_t SearchXMLTag(const uint8_t *xml, uint32_t xmllen, uint32_t *endpos, const uint8_t *tagname, struct stXmlElement *element)
{
    int32_t ret = XML_PARSE_RET_NOT_FOUND;                          /* 解析結果                         */
    uint8_t tmptag[XML_TAG_LEN] = {0};                              /* タグ解析用バッファ               */
    uint8_t attrname[XML_ATTR_LEN] = {0};                           /* 属性名 解析用バッファ            */
    uint8_t attrval[XML_ATTR_LEN] = {0};                            /* 属性値 解析用バッファ            */
    uint32_t bufpos = 0;                                            /* バッファ解析位置                 */
    uint32_t tagpos = 0;                                            /* 開始タグ・終了タグ解析位置       */
    uint32_t attrpos = 0;                                           /* 属性名・属性値解析位置           */
    uint32_t valpos = 0;                                            /* 値解析位置                       */
    enum XML_PARSE_STATUS status = XML_PARSE_STATUS_WAIT_STAG_OPEN; /* 解析状態                         */

    memset(element, 0, sizeof(struct stXmlElement));

    while((bufpos < xmllen) && (ret == XML_PARSE_RET_NOT_FOUND)) {
        switch (status) {
        case XML_PARSE_STATUS_WAIT_STAG_OPEN:                                   /* 開始タグ < 待ち  */
            if (xml[bufpos] == '<') {
                status = XML_PARSE_STATUS_WAIT_STAG_CLOSE;
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_CLOSE:                                  /* 終了タグ > 待ち  */
            if (xml[bufpos] == '>') {
                /* 開始タグ > 検出 */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    strncpy((char *)element->tagname, (char *)tmptag, XML_TAG_LEN);

                    status = XML_PARSE_STATUS_WAIT_ETAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                } else {
                    /* 探しているタグでない → 次のタグを探す */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;

                    /* タグ名解析領域を初期化 */
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            } else if (xml[bufpos] == ' ') {
                memset(attrname, 0, XML_ATTR_LEN);
                memset(attrval, 0, XML_ATTR_LEN);
                attrpos = 0;

                /* 探しているタグかチェック */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    status = XML_PARSE_STATUS_WAIT_STAG_ATTR_SSTR;
                } else {
                    /* 対象外のタグ */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;
                }
            } else {
                tmptag[tagpos++] = xml[bufpos];
                if (256 < tagpos) {
                    /* タグ名がバッファサイズより長い(想定外) → 開始タグの検索に戻る */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_SSTR:                          /* 開始タグ 属性 文字列開始待ち */
            if (xml[bufpos] == '>') {
                /* 開始タグ検出(属性がないまま) */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    strncpy((char *)element->tagname, (char *)tmptag, XML_TAG_LEN);

                    status = XML_PARSE_STATUS_WAIT_ETAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                } else {
                    /* 探しているタグでない → 次のタグを探す */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;

                    /* タグ名解析領域を初期化 */
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            } else if (xml[bufpos] == ' ') {
                /* スペースはスキップ */
            } else {
                if (element->attrcnt < XML_ATTR_CNT) {
                    /* 属性名検出 */
                    status = XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUAL;
                    attrname[attrpos++] = xml[bufpos];
                } else {
                    /* 属性バッファ数より多い数の属性を検出 → 以降の属性は無視する */
                    /* 例). XML_ATTR_CNT が 2の場合に 以下のようなタグを検出
                     *      <hoge id1="111" id2="222" id3="333" idn="xxx">
                     */
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUAL:                             /* 開始タグ 属性 = 待ち */
            if (xml[bufpos] == '=') {
                status = XML_PARSE_STATUS_WAIT_STAG_ATTR_SQUOTE;

                /* 属性値要バッファ初期化 */
                memset(attrval, 0, XML_ATTR_LEN);
                attrpos = 0;
            } else {
                attrname[attrpos++] = xml[bufpos];
                if (XML_ATTR_LEN < attrpos) {
                    /* 属性バッファ数より多い数の属性を検出 → 以降の属性は無視する */
                    /* 例). XML_ATTR_CNT が 2の場合に 以下のようなタグを検出
                     *      <hoge id1="111" id2="222" id3="333" idn="xxx">
                     */
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_SQUOTE:                            /* 開始タグ 属性 開始引用符 待ち */
            if ((xml[bufpos] == '\"') || (xml[bufpos] == '\'')) {
                status = XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUOTE;
            } else {
                /* 属性値が引用符で囲まれていない→不正なXML */
                ret = XML_PARSE_RET_INVALID;    /* 不正なXML */
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUOTE:                            /* 開始タグ 属性 終了引用符 待ち */
            if ((xml[bufpos] == '\"') || (xml[bufpos] == '\'')) {
                strncpy((char *)element->attr[element->attrcnt].attrname, (char *)attrname, XML_ATTR_LEN);
                strncpy((char *)element->attr[element->attrcnt].attrval, (char *)attrval, XML_ATTR_LEN);
                element->attrcnt++;
                attrpos = 0;

                /* 属性 終了引用符検出(正常) */
                status = XML_PARSE_STATUS_WAIT_STAG_CLOSE;
            } else {
                attrval[attrpos++] = xml[bufpos];
                if (XML_ATTR_LEN < attrpos) {
                    /* 属性名がバッファサイズ以上 */
                    ret = XML_PARSE_RET_RES;        /* リソース不足 */
                    assert(0);
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_ETAG_OPEN:
            if (xml[bufpos] == '<') {
                status = XML_PARSE_STATUS_WAIT_ETAG_SLASH;
            }
            /* 終了タグが一致するまでは値として保持しておく */
            element->val[valpos++] = xml[bufpos];
            if (XML_VAL_LEN < valpos) {
                /* タグの値がバッファサイズ以上 */
                ret = XML_PARSE_RET_RES;        /* リソース不足 */
                assert(0);
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_ETAG_SLASH:
            if (xml[bufpos] == '/') {
                status = XML_PARSE_STATUS_WAIT_ETAG_CLOSE;
            }
            /* 終了タグが一致するまでは値として保持しておく */
            element->val[valpos++] = xml[bufpos];
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_ETAG_CLOSE:
            if (xml[bufpos] == '>') {
                /* 終了タグ検出 */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    /* 正常検出 終了タグ部削除 */
                    valpos -= 2;                    /* 検索タグの </ を値から除去   */
                    valpos -= tagpos;               /* 検索タグの タグ文字列を除去  */
                    element->val[valpos] = '\0';

                    /* 指定されたタグを検出 */
                    ret = XML_PARSE_RET_FOUND;
                } else {
                    /* 検索中のタグの終了タグでない →タグのネストなので処理継続 */
                    element->val[valpos++] = xml[bufpos];
                    if (XML_VAL_LEN < valpos) {
                        /* タグの値がバッファサイズ以上 */
                        ret = XML_PARSE_RET_RES;        /* リソース不足 */
                        assert(0);
                    }

                    status = XML_PARSE_STATUS_WAIT_ETAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            } else {
                /* 終了タグが一致するまでは値として保持しておく */
                element->val[valpos++]   = xml[bufpos];
                tmptag[tagpos++] = xml[bufpos];
                if (256 < tagpos) {
                    /* タグ名がバッファサイズより長い(想定外) → 開始タグの検索に戻る */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            }
            bufpos++;
            break;
        default:
            assert(0);
            return XML_PARSE_RET_NOT_FOUND;
        }
    }

    *endpos = bufpos;
    return ret;
}
