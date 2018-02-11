#ifndef __XML_H__
#define __XML_H__


#define XML_TAG_LEN			(256)									/* �^�O��������					*/
#define XML_VAL_LEN			(256)									/* �^�O�v�f������					*/
#define XML_ATTR_CNT		(3)										/* ��������							*/
#define XML_ATTR_LEN		(256)									/* ����������(���́E�l)			*/

/* XML ������� */
struct stXmlAttr {
	uint8_t	attrname[XML_ATTR_LEN];									/* ������							*/
	uint8_t	attrval[XML_ATTR_LEN];									/* �����l							*/
};

/* XML �v�f���*/
struct stXmlElement {
	uint8_t				tagname[XML_TAG_LEN];						/* �^�O��							*/
	uint32_t			attrcnt;									/* ������							*/
	struct stXmlAttr	attr[XML_ATTR_CNT];							/* ����								*/
	uint8_t				val[XML_VAL_LEN];							/* �v�f���e							*/
};

#define XML_PARSE_RET_FOUND			(1)								/* �w��^�O���o						*/
#define XML_PARSE_RET_NOT_FOUND		(0)								/* �w��^�O�����o					*/
#define XML_PARSE_RET_INVALID		(-1)							/* �s����XML						*/
#define XML_PARSE_RET_RES			(-2)							/* ��͗p���\�[�X�s��				*/

extern int32_t SearchXMLTag(const uint8_t *xml, uint32_t xmllen, uint32_t *endpos, const uint8_t *tagname, struct stXmlElement *element);

#endif	/* __XML_H__ */
