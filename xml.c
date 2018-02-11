#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "xml.h"

/* XML ��͏�� */
enum XML_PARSE_STATUS {
    XML_PARSE_STATUS_WAIT_STAG_OPEN,                                /* �J�n�^�O < �҂�                  */
    XML_PARSE_STATUS_WAIT_STAG_CLOSE,                               /* �J�n�^�O > �҂�                  */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_SSTR,                           /* �J�n�^�O ���� �J�n���� �҂�      */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUAL,                          /* �J�n�^�O ���� = �҂�             */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_SQUOTE,                         /* �J�n�^�O ���� �J�n���p�� �҂�    */
    XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUOTE,                         /* �J�n�^�O ���� �I�����p�� �҂�    */
    XML_PARSE_STATUS_WAIT_ETAG_OPEN,                                /* �I���^�O < �҂�                  */
    XML_PARSE_STATUS_WAIT_ETAG_SLASH,                               /* �I���^�O / �҂�                  */
    XML_PARSE_STATUS_WAIT_ETAG_CLOSE                                /* �I���^�O > �҂�                  */
};

int32_t SearchXMLTag(const uint8_t *xml, uint32_t xmllen, uint32_t *endpos, const uint8_t *tagname, struct stXmlElement *element)
{
    int32_t ret = XML_PARSE_RET_NOT_FOUND;                          /* ��͌���                         */
    uint8_t tmptag[XML_TAG_LEN] = {0};                              /* �^�O��͗p�o�b�t�@               */
    uint8_t attrname[XML_ATTR_LEN] = {0};                           /* ������ ��͗p�o�b�t�@            */
    uint8_t attrval[XML_ATTR_LEN] = {0};                            /* �����l ��͗p�o�b�t�@            */
    uint32_t bufpos = 0;                                            /* �o�b�t�@��͈ʒu                 */
    uint32_t tagpos = 0;                                            /* �J�n�^�O�E�I���^�O��͈ʒu       */
    uint32_t attrpos = 0;                                           /* �������E�����l��͈ʒu           */
    uint32_t valpos = 0;                                            /* �l��͈ʒu                       */
    enum XML_PARSE_STATUS status = XML_PARSE_STATUS_WAIT_STAG_OPEN; /* ��͏��                         */

    memset(element, 0, sizeof(struct stXmlElement));

    while((bufpos < xmllen) && (ret == XML_PARSE_RET_NOT_FOUND)) {
        switch (status) {
        case XML_PARSE_STATUS_WAIT_STAG_OPEN:                                   /* �J�n�^�O < �҂�  */
            if (xml[bufpos] == '<') {
                status = XML_PARSE_STATUS_WAIT_STAG_CLOSE;
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_CLOSE:                                  /* �I���^�O > �҂�  */
            if (xml[bufpos] == '>') {
                /* �J�n�^�O > ���o */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    strncpy((char *)element->tagname, (char *)tmptag, XML_TAG_LEN);

                    status = XML_PARSE_STATUS_WAIT_ETAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                } else {
                    /* �T���Ă���^�O�łȂ� �� ���̃^�O��T�� */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;

                    /* �^�O����͗̈�������� */
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            } else if (xml[bufpos] == ' ') {
                memset(attrname, 0, XML_ATTR_LEN);
                memset(attrval, 0, XML_ATTR_LEN);
                attrpos = 0;

                /* �T���Ă���^�O���`�F�b�N */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    status = XML_PARSE_STATUS_WAIT_STAG_ATTR_SSTR;
                } else {
                    /* �ΏۊO�̃^�O */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;
                }
            } else {
                tmptag[tagpos++] = xml[bufpos];
                if (256 < tagpos) {
                    /* �^�O�����o�b�t�@�T�C�Y��蒷��(�z��O) �� �J�n�^�O�̌����ɖ߂� */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_SSTR:                          /* �J�n�^�O ���� ������J�n�҂� */
            if (xml[bufpos] == '>') {
                /* �J�n�^�O���o(�������Ȃ��܂�) */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    strncpy((char *)element->tagname, (char *)tmptag, XML_TAG_LEN);

                    status = XML_PARSE_STATUS_WAIT_ETAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                } else {
                    /* �T���Ă���^�O�łȂ� �� ���̃^�O��T�� */
                    status = XML_PARSE_STATUS_WAIT_STAG_OPEN;

                    /* �^�O����͗̈�������� */
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            } else if (xml[bufpos] == ' ') {
                /* �X�y�[�X�̓X�L�b�v */
            } else {
                if (element->attrcnt < XML_ATTR_CNT) {
                    /* ���������o */
                    status = XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUAL;
                    attrname[attrpos++] = xml[bufpos];
                } else {
                    /* �����o�b�t�@����葽�����̑��������o �� �ȍ~�̑����͖������� */
                    /* ��). XML_ATTR_CNT �� 2�̏ꍇ�� �ȉ��̂悤�ȃ^�O�����o
                     *      <hoge id1="111" id2="222" id3="333" idn="xxx">
                     */
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUAL:                             /* �J�n�^�O ���� = �҂� */
            if (xml[bufpos] == '=') {
                status = XML_PARSE_STATUS_WAIT_STAG_ATTR_SQUOTE;

                /* �����l�v�o�b�t�@������ */
                memset(attrval, 0, XML_ATTR_LEN);
                attrpos = 0;
            } else {
                attrname[attrpos++] = xml[bufpos];
                if (XML_ATTR_LEN < attrpos) {
                    /* �����o�b�t�@����葽�����̑��������o �� �ȍ~�̑����͖������� */
                    /* ��). XML_ATTR_CNT �� 2�̏ꍇ�� �ȉ��̂悤�ȃ^�O�����o
                     *      <hoge id1="111" id2="222" id3="333" idn="xxx">
                     */
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_SQUOTE:                            /* �J�n�^�O ���� �J�n���p�� �҂� */
            if ((xml[bufpos] == '\"') || (xml[bufpos] == '\'')) {
                status = XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUOTE;
            } else {
                /* �����l�����p���ň͂܂�Ă��Ȃ����s����XML */
                ret = XML_PARSE_RET_INVALID;    /* �s����XML */
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_STAG_ATTR_EQUOTE:                            /* �J�n�^�O ���� �I�����p�� �҂� */
            if ((xml[bufpos] == '\"') || (xml[bufpos] == '\'')) {
                strncpy((char *)element->attr[element->attrcnt].attrname, (char *)attrname, XML_ATTR_LEN);
                strncpy((char *)element->attr[element->attrcnt].attrval, (char *)attrval, XML_ATTR_LEN);
                element->attrcnt++;
                attrpos = 0;

                /* ���� �I�����p�����o(����) */
                status = XML_PARSE_STATUS_WAIT_STAG_CLOSE;
            } else {
                attrval[attrpos++] = xml[bufpos];
                if (XML_ATTR_LEN < attrpos) {
                    /* ���������o�b�t�@�T�C�Y�ȏ� */
                    ret = XML_PARSE_RET_RES;        /* ���\�[�X�s�� */
                    assert(0);
                }
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_ETAG_OPEN:
            if (xml[bufpos] == '<') {
                status = XML_PARSE_STATUS_WAIT_ETAG_SLASH;
            }
            /* �I���^�O����v����܂ł͒l�Ƃ��ĕێ����Ă��� */
            element->val[valpos++] = xml[bufpos];
            if (XML_VAL_LEN < valpos) {
                /* �^�O�̒l���o�b�t�@�T�C�Y�ȏ� */
                ret = XML_PARSE_RET_RES;        /* ���\�[�X�s�� */
                assert(0);
            }
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_ETAG_SLASH:
            if (xml[bufpos] == '/') {
                status = XML_PARSE_STATUS_WAIT_ETAG_CLOSE;
            }
            /* �I���^�O����v����܂ł͒l�Ƃ��ĕێ����Ă��� */
            element->val[valpos++] = xml[bufpos];
            bufpos++;
            break;
        case XML_PARSE_STATUS_WAIT_ETAG_CLOSE:
            if (xml[bufpos] == '>') {
                /* �I���^�O���o */
                if (strncmp((char *)tmptag, (char *)tagname, XML_TAG_LEN) == 0) {
                    /* ���팟�o �I���^�O���폜 */
                    valpos -= 2;                    /* �����^�O�� </ ��l���珜��   */
                    valpos -= tagpos;               /* �����^�O�� �^�O�����������  */
                    element->val[valpos] = '\0';

                    /* �w�肳�ꂽ�^�O�����o */
                    ret = XML_PARSE_RET_FOUND;
                } else {
                    /* �������̃^�O�̏I���^�O�łȂ� ���^�O�̃l�X�g�Ȃ̂ŏ����p�� */
                    element->val[valpos++] = xml[bufpos];
                    if (XML_VAL_LEN < valpos) {
                        /* �^�O�̒l���o�b�t�@�T�C�Y�ȏ� */
                        ret = XML_PARSE_RET_RES;        /* ���\�[�X�s�� */
                        assert(0);
                    }

                    status = XML_PARSE_STATUS_WAIT_ETAG_OPEN;
                    memset(tmptag, 0, sizeof(tmptag));
                    tagpos = 0;
                }
            } else {
                /* �I���^�O����v����܂ł͒l�Ƃ��ĕێ����Ă��� */
                element->val[valpos++]   = xml[bufpos];
                tmptag[tagpos++] = xml[bufpos];
                if (256 < tagpos) {
                    /* �^�O�����o�b�t�@�T�C�Y��蒷��(�z��O) �� �J�n�^�O�̌����ɖ߂� */
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
