/*<FH+>************************************************************************/
/*                                                                            */
/* 版权所有: Copyright (C) 烽鸟出行. 2019. All rights reserved.              */
/*                                                                            */
/* 文件名称: Fn_mlist.h                                                       */
/* 内容摘要: 一种链表实现的头文件                                            */
/* 其它说明: 该链表的形式如下所示：                                          */
/*           ================================================================ */
/*               DATA STRUCT                              DATA STRUCT         */
/*              o----------o<-+           LE           +->o----------o        */
/*              |  NODE    |  |         (ROOT)         |  |  NODE    |        */
/*              |  o----o  |  |         o----o         |  |  o----o  |        */
/*           +--|->|next|--|--|-------->|next|---------|--|->|next|--|--+     */
/*        +--|--|--|prev|<-|--|---------|prev|<--------|--|--|prev|<-|--|--+  */
/*        |  |  |  |self|--|--+         |self|->NULL   +--|--|self|  |  |  |  */
/*        |  |  |  o----o  |            o----o            |  o----o  |  |  |  */
/*        |  |  o----------o                              o----------o  |  |  */
/*        |  |                                                          |  |  */
/*        |  |                                                          |  |  */
/*        |  |   DATA STRUCT          DATA STRUCT        DATA STRUCT    |  |  */
/*        |  |  o----------o<-+      o----------o<-+   +->o----------o  |  |  */
/*        |  |  |  NODE    |  |      |  NODE    |  |   |  |  NODE    |  |  |  */
/*        |  |  |  o----o  |  |      |  o----o  |  |   |  |  o----o  |  |  |  */
/*        |  +--|--|next|<-|--|------|--|next|<-|--|---|--|--|next|<-|--+  |  */
/*        +-----|->|prev|--|--|------|->|prev|--|--|---|--|->|prev|--|-----+  */
/*              |  |self|--|--+      |  |self|--|--+   +--|--|self|  |        */
/*              |  o----o  |         |  o----o  |         |  o----o  |        */
/*              o----------o         o----------o         o----------o        */
/*           ================================================================ */
/*                                                                            */
/*           使用举例如下:                                                   */
/*           ================================================================ */
/*           typedef struct t_data                                            */
/*           {                                                                */
/*              int         id;                                               */
/*              T_LIST_NODE tListNode;                                        */
/*           } T_DATA;                                                        */
/*                                                                            */
/*           1.定义根节点并初始化                                            */
/*              T_LIST_ROOT(gtListRoot);                                      */
/*              LIST_INIT_ROOT(gtListRoot);                                   */
/*                                                                            */
/*           2.构造链表节点P                                                  */
/*              p = (T_DATA *)malloc(sizeof(T_DATA));                         */
/*              p->id = xx;                                                   */
/*              LIST_INIT_NOTE(p->tListNode, p);                              */
/*                                                                            */
/*           3.插入节点p                                                      */
/*              LIST_INSERT_AFTER(gtListRoot, p->tListNode);  // 插入根节点后 */
/*              LIST_INSERT_BEFORE(gtListRoot, p->tListNode); // 插入根节点前 */
/*                                                                            */
/*           4.删除节点p                                                      */
/*              LIST_REMOVE(p->tListNode);                                    */
/*              free(p);                                                      */
/*                                                                            */
/*           5.链表查找                                                       */
/*              注意:根节点是没有数据的,那么以根节点为标杆,通过如下宏定义:  */
/*              LIST_NEXT_IN(gtListRoot);                                     */
/*              LIST_PREV_IN(gtListRoot);                                     */
/*              即可找到起始数据，并通过这两个宏一直找下去，直到返回NULL；  */
/*              因为最后找到的是根节点，而根节点的self字段永远是空          */
/*           ================================================================ */
/* 当前版本: v1.0                                                             */
/* 作    者:                                                                  */
/* 完成日期: 2020-11-19                                                       */
/* 修改记录:                                                                  */
/*     修改日期          版本号      修改人          修改内容                */
/* -------------------------------------------------------------------------- */
/*     2020-11-19        v 1.0.0     Leonard         创建文件                 */
/*<FH->************************************************************************/
#ifndef FN_MLIST_H
#define FN_MLIST_H

/******************************************************************************/
/*               #include（依次为标准库头文件、非标准库头文件）             */
/******************************************************************************/
#include <stddef.h>

/******************************************************************************/
/*                              其他条件编译选项                             */
/******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                  常量定义                                  */
/******************************************************************************/


/******************************************************************************/
/*                                 全局宏定义                                 */
/******************************************************************************/
/******************************************************************************/
/* 链表根节点初始化，R链表根节点                                             */
/******************************************************************************/
#define LIST_INIT_ROOT(R)                   (R).self = NULL;                   \
                                            (R).next = &(R);                   \
                                            (R).prev = &(R)

/******************************************************************************/
/* 链表节点初始化，N:链表的节点，S:链表节点所在的结构体指针                 */
/******************************************************************************/
#define LIST_INIT_NOTE(N, S)                (N).self = (S);                    \
                                            (N).next = NULL;                   \
                                            (N).prev = NULL

/******************************************************************************/
/* 判断节点是否是根节点                                                      */
/******************************************************************************/
#define LIST_IS_ROOT(R)                     ((R).self == NULL)

/******************************************************************************/
/* 判断链表是否为空                                                          */
/******************************************************************************/
#define LIST_IS_EMPTY(R)                    ((R).next == &(R))

/******************************************************************************/
/* 判断节点是否在链表中                                                      */
/******************************************************************************/
#define LIST_IS_IN(N)                       ((N).next != NULL)

/******************************************************************************/
/* 链表对应节点的下一个节点                                                  */
/******************************************************************************/
#define LIST_NEXT_IN(N)                     (void *)((N).next->self)

/******************************************************************************/
/* 链表对应节点的前一个节点                                                  */
/******************************************************************************/
#define LIST_PREV_IN(N)                     (void *)((N).prev->self)

/******************************************************************************/
/* 将节点N插入到节点P的后面                                                  */
/******************************************************************************/
#define LIST_INSERT_AFTER(P, N)             (N).next = (P).next;               \
                                            (N).prev = &(P);                   \
                                            (N).next->prev = &(N);             \
                                            (N).prev->next = &(N)

/******************************************************************************/
/* 将节点N插入到节点P的前面                                                  */
/******************************************************************************/
#define LIST_INSERT_BEFORE(P, N)            (N).prev = (P).prev;               \
                                            (N).next = &(P);                   \
                                            (N).next->prev = &(N);             \
                                            (N).prev->next = &(N)

/******************************************************************************/
/* 将节点N从链表中删除                                                        */
/******************************************************************************/
#define LIST_REMOVE(N)                      (N).next->prev = (N).prev;         \
                                            (N).prev->next = (N).next;         \
                                            (N).next = NULL;                   \
                                            (N).prev = NULL

/******************************************************************************/
/*                              全局数据类型定义                             */
/******************************************************************************/
/*<STRUCT+>********************************************************************/
/* 结构: T_LIST_NODE                                                          */
/* 注释: 链表节点元素（list element）的结构定义                              */
/*<STRUCT->********************************************************************/
typedef struct t_list_node
{
    struct t_list_node *next;   /* 指向下一链表元素的指针                     */
    struct t_list_node *prev;   /* 指向前一链表元素的指针                     */
    void               *self;   /* 指向包含本链表元素的结构的起始地址的指针  */

} T_LIST_NODE, T_LIST_ROOT;


/******************************************************************************/
/*                                全局变量声明                                */
/******************************************************************************/


/******************************************************************************/
/*                                 全局函数原型                               */
/******************************************************************************/


#ifdef __cplusplus
}
#endif

#endif



