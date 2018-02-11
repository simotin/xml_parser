#ifndef __XML_H__
#define __XML_H__


#define XML_TAG_LEN			(256)									/* タグ名文字列長					*/
#define XML_VAL_LEN			(256)									/* タグ要素文字列長					*/
#define XML_ATTR_CNT		(3)										/* 属性件数							*/
#define XML_ATTR_LEN		(256)									/* 属性文字列長(名称・値)			*/

/* XML 属性情報 */
struct stXmlAttr {
	uint8_t	attrname[XML_ATTR_LEN];									/* 属性名							*/
	uint8_t	attrval[XML_ATTR_LEN];									/* 属性値							*/
};

/* XML 要素情報*/
struct stXmlElement {
	uint8_t				tagname[XML_TAG_LEN];						/* タグ名							*/
	uint32_t			attrcnt;									/* 属性数							*/
	struct stXmlAttr	attr[XML_ATTR_CNT];							/* 属性								*/
	uint8_t				val[XML_VAL_LEN];							/* 要素内容							*/
};

#define XML_PARSE_RET_FOUND			(1)								/* 指定タグ検出						*/
#define XML_PARSE_RET_NOT_FOUND		(0)								/* 指定タグ未検出					*/
#define XML_PARSE_RET_INVALID		(-1)							/* 不正なXML						*/
#define XML_PARSE_RET_RES			(-2)							/* 解析用リソース不足				*/

extern int32_t SearchXMLTag(const uint8_t *xml, uint32_t xmllen, uint32_t *endpos, const uint8_t *tagname, struct stXmlElement *element);

#endif	/* __XML_H__ */
