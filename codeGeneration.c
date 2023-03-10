#include "stdio.h"
#include "codeGeneration.h"

AttributeNode* lookupAttribute(AttributeNode *head, enum AttributeType type){
    AttributeNode *p = head;
    while (p != NULL && p->type != type)
        p = p->next;
    if (p == NULL)
        return NULL;
    else
        return p;
}

AttributeNode *installAttribute(AttributeNode *attribute, AttributeNode *head){
    if (head == NULL)
        head =  attribute;
    else
    {
        AttributeNode *p = lookupAttribute(head, attribute.type);
        if (p == NULL)
            p->next = attribute;    // no such attribute, add it
        else
            p->val = attribute->val;    // attribute exists, renew it
    }
    return head;
}

