#ifndef _TANGY_HATCH_H_
#define _TANGY_HATCH_H_

#define HT_NONE             (0)
#define HT_SLASHED          (1)
#define HT_BACKSLASHED      (2)
#define HT_HLINED           (3)
#define HT_VLINED           (4)
#define HT_CROSSED          (5)
#define HT_XCROSSED         (6)
#define HT_SOLID            (10)
#define HT_SPARSEDOTTED     (14)
#define HT_VZIGZAG          (15)
#define HT_HZIGZAG          (16)
#define HT_CHECKED          (17)
#define HT_BIGCHECKED       (18)
#define HT_HEX              (20)
#define HT_BRICK            (21)
#define HT_DIAMONDPLATE     (26)
#define HT_NFDIAMONDPLATE   (27)

#define HT_CONCENTRATION    (28)
#define HT_CONCENTRIC       (29)

#define HT_DOTTED           (40)
#define HT_DOT1             (41)
#define HT_DOT2             (42)
#define HT_DOT3             (43)
#define HT_DOT4             (44)
#define HT_DOT5             (45)
#define HT_DOT6             (46)
#define HT_DOT7             (47)
#define HT_DOT8             (48)
#define HT_DOT9             (49)

#define HT_GRID             (50)

#define HT_UGRID005         (51)
#define HT_UGRID010         (52)
#define HT_UGRID020         (53)
#define HT_UGRID025         (54)
#define HT_UGRID050         (55)
#define HT_UGRID100         (56)    /* unit pitch grid */
#define HT_UGRID200         (57)

#define HT_NCGRID           (59)    /* non-centered grid */

#define HT_HSTRIPE          (61)
#define HT_VSTRIPE          (62)
#define HT_HVSTRIPE         (63)
#define HT_PLUSSTRIPE       (64)

#define HT_AVLINEE          (66)
#define HT_AVLINEW          (67)
#define HT_AHLINEN          (68)
#define HT_AHLINES          (69)

#define HT_MTN              (71)
#define HT_MTFN             (72)
#define HT_MT1N             (73)

#define HT_MTS              (76)
#define HT_MTFS             (77)
#define HT_MT1S             (78)

#define HT_MTE              (81)
#define HT_MTFE             (82)
#define HT_MT1E             (83)

#define HT_MTW              (86)
#define HT_MTFW             (87)
#define HT_MT1W             (88)


#define HT_HISHI            (101)
#define HT_HIGAKI           (102)
#define HT_RAIMON           (103)
#define HT_SEIGAIHA         (104)
#define HT_UROKO            (105)
#define HT_NFUROKO          (106)
#define HT_SHIPPO           (107)
#define HT_NFSHIPPO         (108)
#define HT_MAMESHIBORI      (109)
#define HT_NFMAMESHIBORI    (110)
#define HT_KANOKOSHIBORI    (111)
#define HT_FUNDO            (120)
#define HT_YAGASURI         (121)

#define HT_SQRING11         (131)
#define HT_SQRING11WRAP     (132)
#define HT_SQRING9          (133)
#define HT_SQRING9WRAP      (134)
#define HT_SQRING7          (135)
#define HT_SQRING7WRAP      (136)
#define HT_MIKUZUSI         (137)
#define HT_RMIKUZUSI        (138)

#define HT_TATEWAKU         (231)
#define HT_FTATEWAKU        (232)
#define HT_YOKOWAKU         (236)
#define HT_FYOKOWAKU        (237)

#define HT_DUMMY            (999)

extern apair_t hatchtype_ial[];
#endif/*_TANGY_HATCH_H_*/
