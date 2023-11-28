/**
 * @brief   the defition of the list interface
 * 
 * @file    aiio_list.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       define some single list interface
 * 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-05-25          <td>1.0.0            <td>zhuolm             <td> list interface
 */
#include "aiio_list.h"
#include "aiio_log.h"




bool aiio_ListInit(aiioList_t *list, uint16_t node_max_num)
{
    if(list== NULL)
    {
        aiio_log_e("list param error \r\n");
        return false;
    }

    list->head = NULL;
    list->node_max_num = node_max_num;
    list->node_num = 0;

    return true;
}


bool aiio_ListAddNode(aiioList_t *list, ListNode_t *node)
{
    ListNode_t *new_node = NULL;
    ListNode_t *tail_node = NULL;

    if (NULL == list || NULL == node)
    {
        aiio_log_e("args error");
        return false;
    }

    if (list->node_max_num <= list->node_num)
    {
        aiio_log_e("node number too big");
        return false;
    }

    new_node = (ListNode_t *)malloc(sizeof(ListNode_t));

    if (NULL == new_node)
    {
        aiio_log_e("malloc error");
        return false;
    }

    new_node->pbase = (uint8_t *)malloc(node->size);
    if (NULL == new_node->pbase)
    {
        aiio_log_e("malloc error");
        free(new_node);
        return false;
    }
    memcpy(new_node->pbase, node->pbase, node->size);
    new_node->next_node = NULL;
    new_node->size = node->size;

    if (NULL == list->head)
    {
        list->head = new_node;
    }
    else
    {
        tail_node = list->head;
        while (tail_node->next_node)
        {
            tail_node = tail_node->next_node;
        }
        tail_node->next_node = new_node;
    }

    list->node_num++;

    return true;
}


bool aiio_ListAddSecondNode(aiioList_t *list, ListNode_t *node)
{
    ListNode_t *new_node = NULL;
    ListNode_t *tail_node = NULL;
    ListNode_t *node_next = NULL;

    if (NULL == list || NULL == node)
    {
        aiio_log_e("args error");
        return false;
    }

    if (list->node_max_num <= list->node_num)
    {
        aiio_log_e("node number too big");
        return false;
    }

    new_node = (ListNode_t *)malloc(sizeof(ListNode_t));

    if (NULL == new_node)
    {
        aiio_log_e("malloc error");
        return false;
    }

    new_node->pbase = (uint8_t *)malloc(node->size);
    if (NULL == new_node->pbase)
    {
        aiio_log_e("malloc error");
        free(new_node);
        return false;
    }
    memcpy(new_node->pbase, node->pbase, node->size);
    new_node->next_node = NULL;
    new_node->size = node->size;

    if (NULL == list->head)
    {
        list->head = new_node;
    }
    else
    {
        tail_node = list->head;
        node_next = tail_node->next_node;
        tail_node->next_node = new_node;
        new_node->next_node = node_next;
    }

    list->node_num++;

    return true;
}



bool aiio_ListDelNode(aiioList_t *list, ListNode_t *node)
{
    ListNode_t *cur_node = NULL;
    ListNode_t *prev_node = NULL;

    if (NULL == list || NULL == node)
    {
        aiio_log_e("args error");
        return false;
    }

    cur_node = list->head;
    
    if (NULL == cur_node || list->node_num == 0)
    {
        aiio_log_e("the list is empty");
        return false;
    }

    while ((cur_node != NULL) && (cur_node->next_node != NULL))
    {
        if (cur_node == node)
        {
            break;
        }
        prev_node = cur_node;
        cur_node = cur_node->next_node;
    }

    if (cur_node == node)
    {
        if (NULL == prev_node)
        {
            list->head = cur_node->next_node;
        }
        else
        {
            prev_node->next_node = cur_node->next_node;
        }
        
        free(cur_node->pbase);
        free(cur_node);
        list->node_num--;

        return true;
    }
    else
    {
        aiio_log_e("Node not found");
        return false;
    }
}



uint16_t aiio_ListGetNodeNum(aiioList_t *list)
{
    if(!list)
    {
        return -1;
    }
    return list->node_num;
}


ListNode_t *aiio_ListGetNodeFromIndex(aiioList_t *list, uint16_t index)
{
    ListNode_t *cur_node = NULL;
    uint16_t num = 0;
    uint16_t i = 0;

    if (NULL == list)
    {
        aiio_log_e("args error");
        return NULL;
    }
    
    num = list->node_num;
    cur_node = list->head;

    if (NULL == cur_node)
    {
        aiio_log_e("the list is empty");
        return NULL;
    }

    if (index >= num)
    {
        aiio_log_e("the list index err");
        return NULL;
    }

    while (cur_node)
    {
        if (i == index)
        {
            return cur_node;
        }
        i++;
        cur_node = cur_node->next_node;
    }
    
    return NULL;
}


