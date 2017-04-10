#include <cstdlib> 
#include "json.h"
#include <string.h>
#include <stdio.h>

JsonNode::JsonNode()
{
  InitVariable();
}

JsonNode::JsonNode(const char *name)
{
  InitVariable();
  if(name != NULL)
  {
    int nameLen = strlen(name);
    namestring = new char[nameLen + 1];
    memcpy(namestring,name,nameLen);
    namestring[nameLen] = 0;
  }
}

JsonNode::JsonNode(const char *name,const char *value)
{
  InitVariable();
  if(name != NULL)
  {
    int nameLen = strlen(name);
    namestring = new char[nameLen + 1];
    memcpy(namestring,name,nameLen);
    namestring[nameLen] = 0;
  }
  
  if(value != NULL)
  {
    int valueLen =  strlen(value);
    nodeValue.valuestring = new char[valueLen + 1];
    memcpy(nodeValue.valuestring,value,valueLen);
    nodeValue.valuestring[valueLen] = 0;
  }
  type = NodeStringType;
}

JsonNode::JsonNode(const char *name,long long value)
{
  InitVariable();
  if(name != NULL)
  {
    int nameLen = strlen(name);
    namestring = new char[nameLen + 1];
    memcpy(namestring,name,nameLen);
    namestring[nameLen] = 0;
  }
  
  nodeValue.valueint = value;
  type = NodeIntType;
}

JsonNode::JsonNode(const char *name,double value)
{
  InitVariable();
  if(name != NULL)
  {
    int nameLen = strlen(name);
    namestring = new char[nameLen + 1];
    memcpy(namestring,name,nameLen);
    namestring[nameLen] = 0;
  }
  
  nodeValue.valuedouble = value;
  type = NodeFloatType;
}

JsonNode::~JsonNode()
{
  if(type == NodeChildType || type == NodeArrayType)
  {
    JsonNode *childNode = this->nodeValue.child;
    while(childNode)
    {
      JsonNode *nextChildNode = childNode->NextNode();
      delete childNode;
      childNode = nextChildNode;
      if(childNode ==  this->nodeValue.child)
        break;
    }
  }
  
  if(type == NodeStringType && this->nodeValue.valuestring != NULL)
  {
    delete[] this->nodeValue.valuestring;
  }

  next = prev = NULL;
  if(namestring != NULL) 
  {
    delete[] namestring;
    namestring = NULL;
  }
  
  type = NodeNullType;
  nodeValue.valueint = 0;
}

JsonNode *JsonNode::AddStringNode(const char *name,const char *value,int index)
{
  //printf("AddStringNode\n");
  return AddSubNode(new JsonNode(name,value),index);
}

JsonNode *JsonNode::AddIntNode(const char *name,long long value,int index)
{
  return AddSubNode(new JsonNode(name,value),index);
}

JsonNode *JsonNode::AddDoubleNode(const char *name,double value,int index)
{
  return AddSubNode(new JsonNode(name,value),index);
}

JsonNode *JsonNode::AddNullNode(const char *name,int index)
{
  if(name == NULL)
    return AddSubNode(new JsonNode(),index);
  else 
    return AddSubNode(new JsonNode(name),index);
}  

JsonNode *JsonNode::AddSubNode(JsonNode *subNode,int index)
{
  //printf("AddSubNode,%x,%x,%x\n",subNode->next,subNode->prev,subNode->parent);
  if(type == NodeIntType || type == NodeFloatType || type == NodeStringType || subNode == NULL) 
    return NULL;

  if(this->type == NodeNullType)
     this->type = NodeChildType;
  
  subNode->parent = this;
  if(this->nodeValue.child == NULL)
  {
    //printf("this->nodeValue.child == NULL\n");
    this->nodeValue.child = subNode;
    return subNode;
  }

  if(index == 0)
  {
     subNode->next = this->nodeValue.child;
     if(this->nodeValue.child != NULL)
       this->nodeValue.child->prev = subNode;
     subNode->prev = NULL;
     this->nodeValue.child = subNode;
  }
  else
  {
     JsonNode *childNode = this->nodeValue.child;
     for(int i = 0 ; i < index ; i ++)
     {
       if(childNode->next != NULL)
          childNode = childNode->next;
       else
          break;
     }
     subNode->next = childNode->next;
     if(childNode->next != NULL)
        childNode->next->prev = subNode;
     subNode->prev = childNode;
     childNode->next = subNode;
  }


   
  return subNode;
}

JsonNode *JsonNode::ChildNode()
{
  if(type == NodeChildType || type == NodeArrayType)
     return this->nodeValue.child;
  else
     return NULL;
}

JsonNode *JsonNode::PrevNode()
{
  return this->prev;
}

JsonNode *JsonNode::NextNode()
{
  return this->next;
}

JsonNode *JsonNode::FindChildNodeName(const char *name)
{
  if(type == NodeChildType)
  {
    JsonNode *childNode = this->nodeValue.child;
    while(childNode)
    {
      if(strcmp(name,childNode->namestring) == 0)
         return childNode;
      childNode = childNode->NextNode();
      if(childNode ==  this->nodeValue.child)
        break;;
    }
  }
  return NULL;
}

JsonNode *JsonNode::FindSubNodeName(const char *name)
{
  if(type == NodeChildType)
  {
    JsonNode *childNode = this->nodeValue.child;
    while(childNode)
    {
      if(strcmp(name,childNode->namestring) == 0)
         return childNode;
      JsonNode *pNode = childNode->FindSubNodeName(name);
      if(pNode)
          return pNode;
      childNode = childNode->NextNode();
      if(childNode ==  this->nodeValue.child)
        break;
    }
  }
  return NULL;
}

int JsonNode::DeleteChildNodeName(const char *name)
{
  //printf("DeleteChildNodeName\n");
  JsonNode*childNode = FindChildNodeName(name);
  if(childNode == NULL)
    return -1;
  return DeleteNode(childNode);
}

int JsonNode::DeleteSubNodeName(const char *name)
{
  JsonNode*childNode = FindSubNodeName(name);
  if(childNode == NULL)
    return -1;
  return DeleteNode(childNode);
}

int JsonNode::DeleteNode(JsonNode *subNode)
{
  //printf("DeleteNode\n");
  if(subNode == NULL)
    return -1;
  if(subNode->prev == NULL) 
  {
    if(subNode->parent != NULL)
    {
       subNode->parent->nodeValue.child = subNode->next;
       if(subNode->next != NULL)
           subNode->next->prev = NULL;
    }       
  }
  else 
  {
    subNode->prev->next = subNode->next;
    if(subNode->next != NULL)
        subNode->next->prev = subNode->prev;
  }
  delete subNode;
  return 0;
}

int JsonNode::SetName(const char *name)
{
  if(namestring != NULL)
  {
    delete []namestring ;
    namestring = NULL;
  }
  
  if(name != NULL)
  {
    int nameLen = strlen(name);
    namestring = new char[nameLen + 1];
    memcpy(namestring,name,nameLen);
    namestring[nameLen] = 0;
  }
  return 0;
}

int JsonNode::SetStringValue(const char *value)
{
  if(type != NodeStringType && type != NodeNullType)
  {
    return -1;
  }
  
  if(nodeValue.valuestring != NULL)
  {
    delete[]nodeValue.valuestring;
    nodeValue.valuestring = NULL;
  }

  if(value != NULL)
  {
    int valueLen =  strlen(value);
    nodeValue.valuestring = new char[valueLen + 1];
    memcpy(nodeValue.valuestring,value,valueLen);
    nodeValue.valuestring[valueLen] = 0;
  }
  return 0;
}

int JsonNode::SetIntValue(long long value)
{
  if(type != NodeIntType && type != NodeNullType)
  {
    return -1;
  } 
  nodeValue.valueint = value;
  return 0;
}

int JsonNode::SetDoubleValue(double value)
{
  if(type != NodeFloatType && type != NodeNullType)
  {
    return -1;
  } 
  nodeValue.valueint = value;
  return 0;
}

int JsonNode::SetValueType(NodeType typeV)
{
  if(this->type == typeV)
     return 0;
  if(this->type == NodeStringType)
  {
    if(nodeValue.valuestring != NULL)
    {
      delete[]nodeValue.valuestring;
      nodeValue.valuestring = NULL;
    }
  }
  else if(this->type == NodeIntType || this->type == NodeFloatType)
  {
    nodeValue.valueint = 0;
  }
  else if(this->type == NodeChildType || this->type == NodeArrayType)
  {
    if(nodeValue.child != NULL)
    {
      delete this->nodeValue.child;
      this->nodeValue.child = NULL;
    }
  }
  this->type = typeV;
  return 0;
}

int JsonNode::GetName(char *name)
{
  if(name == NULL)
     return -1;
  if(namestring != NULL)
  {
    int nameLen = strlen(namestring);
    memcpy(name,namestring,nameLen);
    name[nameLen] = 0;
    return nameLen;
  }
  name[0] = 0;
  return 0;
}

int JsonNode::GetStringValue(char *value)
{
  if(value == NULL)
     return -1;
  if(type != NodeStringType)
     return -1;
 
  if(nodeValue.valuestring != NULL)
  {
    int len = strlen(nodeValue.valuestring);
    memcpy(value,nodeValue.valuestring,len);
    value[len] = 0;
    return len;
  }
  value[0] = 0;
  return 0;
}

int JsonNode::GetIntValue(long long *value)
{
  if(value == NULL)
     return -1;
  if(type != NodeIntType)
     return -1;
 
  *value = nodeValue.valueint;
  return 0;
}

int JsonNode::GetDoubleValue(double *value)
{
  if(value == NULL)
     return -1;
  if(type != NodeFloatType)
     return -1;
 
  *value = nodeValue.valuedouble;
  return 0;
}

NodeType JsonNode::GetValueType(void)
{
  return type;
}

int JsonNode::ToString(char *buff,int size)
{
  //printf("ToString\n");
  int index = 0;
  int len = 0;

  if(buff == NULL)
  {
    return -1;
  }
   
  //printf("type = %d,",this->type);
  if(namestring != NULL)
  {
    //printf("namestring = %s",namestring);
    len = strlen(namestring);
    buff[index ++] = '\"';
    memcpy(buff + index,namestring,len);
    index += len;
    buff[index ++] = '\"';
    buff[index ++] = ':';
  }
  //printf("\n");

  if(this->type == NodeStringType)
  {
    //printf("this->type == NodeStringType\n");
    if(nodeValue.valuestring != NULL)
    {
      len = strlen(nodeValue.valuestring);
      buff[index ++] = '\"';
      memcpy(buff + index,nodeValue.valuestring,len);
      index += len;
      buff[index ++] = '\"';
    }
  }
  else if(this->type == NodeIntType)
  {
    index += sprintf(buff + index,"%lld",nodeValue.valueint);
  }
  else if(this->type == NodeFloatType)
  {
    index += sprintf(buff + index,"%f",nodeValue.valuedouble);
  }
  else if(this->type == NodeChildType)
  {
    //printf("this->type == NodeChildType\n");
    buff[index ++] = '{';
    //buff[index ++] = '\n';
    JsonNode *childNode = this->nodeValue.child;
    while(1)
    {
      index += childNode->ToString(buff + index,size - index);
      childNode = childNode->next;
      if(childNode == NULL || childNode == this->nodeValue.child) 
      {
        //buff[index ++] = '\n';
        break;
      } 
      else 
      {
        buff[index ++] = ',';
        //buff[index ++] = '\n';
      }
    }
    buff[index ++] = '}';
  }
  else if(this->type == NodeArrayType)
  {
    buff[index ++] = '[';
    JsonNode *childNode = this->nodeValue.child;
    while(1)
    {
      index += childNode->ToString(buff + index,size - index);
      childNode = childNode->next;
      if(childNode == NULL || childNode == this->nodeValue.child) 
      {
        break;
      } 
      else 
      {
        buff[index ++] = ',';
      }
    }
    buff[index ++] = ']';
  }
  else if(this->type == NodeNullType)
  {
    memcpy(buff + index,"null",4);
    index += 4;
  }
  buff[index] = 0;
 // printf("index =%d\n",index);
  return index;
}

int JsonNode::LoadFromString(const char *buff,int size)
{
  //printf("LoadFromString\n");
  int index = 0;
  int step = 0;
  if(size <= 0)
  {
    while(buff[index])
    {
      //printf("index = %d,buff[index]=%c,step = %d\n",index,buff[index],step);
      switch(buff[index])
      {
        case '\"':
          if(step == 0)
          {
            index += FindAndMallocStr(buff + index,&namestring,size - index);
            step ++;
          }
          else if(step == 2)
          {
            index += FindAndMallocStr(buff + index,&nodeValue.valuestring,size - index);
            type = NodeStringType;
            return index;
          }
          break;
        case '{':
          if(step == 0)
          {
            step = 2;
            index ++;
            JsonNode *pNode = this->AddNullNode(NULL,0xfffffff);
            index += pNode->LoadFromString(buff + index , size - index);
            type = NodeChildType;
          }
          else if(step == 2)
          {
            index ++;
            JsonNode *pNode = this->AddNullNode(NULL,0xfffffff);
            index += pNode->LoadFromString(buff + index , size - index);
            type = NodeChildType;
          }
          break;
        case '[':
          if(step == 0)
          {
            step = 2;
            index ++;
            JsonNode *pNode = this->AddNullNode(NULL,0xfffffff);
            index += pNode->LoadFromString(buff + index , size - index);
            type = NodeArrayType;
          }
          else if(step == 2)
          {
            index ++;
            JsonNode *pNode = this->AddNullNode(NULL,0xfffffff);
            index += pNode->LoadFromString(buff + index , size - index);
            type = NodeArrayType;
          }
          break;
        case ':':
          if(step == 1)
          {
             index ++;
             step ++;   
          }
          break;
        case ']':
        case '}':
          if(step == 2)
          {
             index ++;
             return index;
          }
          else if(step == 1)
          {
            if(namestring != NULL)
            {
               nodeValue.valuestring = namestring;
               type = NodeStringType;
               namestring = NULL;
            }
            return index;
          }
          break;
        case ',':
          if(step == 2)
          {
            if(type == NodeArrayType || type == NodeChildType)
            {
              index ++;
              JsonNode *pNode = this->AddNullNode(NULL,0xfffffff);
              index += pNode->LoadFromString(buff + index , size - index);
            }
          }
          else if(step == 1)
          {
            if(namestring != NULL)
            {
               nodeValue.valuestring = namestring;
               type = NodeStringType;
               namestring = NULL;
               return index;
            }
          }
          break;
        default:
          if(buff[index] <= '9' && buff[index] >= '0')
          {
            index += FindAndSetNumberStr(buff + index,size - index);
            return index;
          }
          else 
            index ++;
          break;
      }
    }
  }
  return index;
}

int JsonNode::FindAndMallocStr(const char *buff,char **outbuff,int size)
{
  //printf("FindAndMallocStr\n");
  int index = 0;
  int begin = 0,end = 0;
  while(1)
  {
    if(buff[index] == '\"')
       break;
    else if(buff[index] == 0)
       return index;
    index ++;
  }
  index ++;
  begin = index;
  while(1)
  {
    if(buff[index] == '\"')
       break;
    else if(buff[index] == 0)
       return index;
    index ++;
  }

  end = index;
  index ++;
  //printf("end - begin = %d\n",end - begin);
  *outbuff = new char[end - begin + 1];
  //printf("*outbuff = new char[end - begin + 1];\n");
  memcpy(*outbuff,buff + begin,end - begin);
  //printf("memcpy(*outbuff,buff,end - begin);\n");
  (*outbuff)[end - begin] = 0;
  //printf(*outbuff) ;
  //printf("\n");
  return index;
}

int JsonNode::FindAndSetNumberStr(const char *buff,int size )
{
  int index = 0;
  //int begin = 0;
  bool dotFlag = false;
  long long sum = 0;
  double devNum = 1;
 
  while(1)
  {
    if(buff[index] <= '9' && buff[index] >= '0')
       break;
    else if(buff[index] == 0)
       return index;
    index ++;
  }
  //begin = index;
  while(1)
  {
    if(buff[index] <= '9' && buff[index] >= '0')
    {
       sum = sum * 10 + buff[index] - '0';
       index ++;
       if(dotFlag)
         devNum /= 10;
    }
    else if(!dotFlag && buff[index] == '.')
    { 
       index ++;
       dotFlag = true;
    }
    else if(buff[index] == 0)
       return index;
    else 
       break;
  }

  if(dotFlag)
  {
    nodeValue.valuedouble = sum * devNum;
    type = NodeFloatType;
  }
  else
  { 
    nodeValue.valueint = sum;
    type = NodeIntType;
  }
    
  return index;
}


void JsonNode::InitVariable(void)
{
    next = prev = parent = NULL;
  namestring = NULL;
  type = NodeNullType;
  nodeValue.valueint = 0;
}


