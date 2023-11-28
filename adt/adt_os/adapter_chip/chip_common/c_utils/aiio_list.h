/**
 * @brief   the declare of the list interface
 * 
 * @file    aiio_list.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       declare some single list interface
 * 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-05-25          <td>1.0.0            <td>zhuolm             <td> list interface
 */
#ifndef __AIIO_LIST_H_
#define __AIIO_LIST_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief   The structure of list node
 */
typedef struct node
{
    uint8_t *pbase;
    uint16_t size;

    struct node *next_node;

}ListNode_t;


/**
 * @brief   The sttructure of the list
 */
typedef struct 
{
    ListNode_t *head;

    uint16_t node_num;
    uint16_t node_max_num;

}aiioList_t;



/** @brief The Initialization of list
 *
 *  @param[in]      list                    The parameter of list
 *  @param[in]      node_max_num            The parameter of the list maximum node
 *  
 *  @return                          Return the bool value status, if true, the list initialize success, otherwise is fail.
 */
bool aiio_ListInit(aiioList_t *list, uint16_t node_max_num);


/** @brief  Add the node data in list
 *
 *  @param[in]      list                    The parameter of list
 *  @param[in]      node                    The parameter of the node data
 *  
 *  @return                          Return the bool value status, if true, the list add node success, otherwise is fail.
 */
bool aiio_ListAddNode(aiioList_t *list, ListNode_t *node);


/** @brief  Add the node data into the second location of list
 *
 *  @param[in]      list                    The parameter of list
 *  @param[in]      node                    The parameter of the node data
 *  
 *  @return                          Return the bool value status, if true, the list add node success, otherwise is fail.
 */
bool aiio_ListAddSecondNode(aiioList_t *list, ListNode_t *node);



/** @brief  Delete the node data from list
 *
 *  @param[in]      list                    The parameter of list
 *  @param[in]      node                    The parameter of the node data
 *  
 *  @return                          Return the bool value status, if true, Delete node from list is success, otherwise is fail.
 */
bool aiio_ListDelNode(aiioList_t *list, ListNode_t *node);

/** @brief  Get the number of nodes in list
 *
 *  @param[in]      list                    The parameter of list
 *  
 *  @return                          Return the number of nodes or 0.
 */
uint16_t aiio_ListGetNodeNum(aiioList_t *list);

/** @brief  Get the data of nodes in list
 *
 *  @param[in]      list                    The parameter of list
 *  @param[in]      index                    The parameter of the node number
 *  
 *  @return                          Return the node data pointer or NULL.
 */
ListNode_t *aiio_ListGetNodeFromIndex(aiioList_t *list, uint16_t index);


#endif
