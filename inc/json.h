
#ifndef __JSON_H
#define __JSON_H

enum NodeType
{
  NodeStringType,
  NodeIntType,
  NodeFloatType,
  NodeChildType,
  NodeArrayType,
  NodeNullType
};

class JsonNode;

union NodeValueUnion
{
  char *valuestring;
  JsonNode *child;
  long long valueint;
  double valuedouble;
};

class JsonNode
{
public:
  //friend class JsonNode;
  JsonNode();
  JsonNode(const char *name);
  JsonNode(const char *name,const char *value);
  JsonNode(const char *name,long long value);
  JsonNode(const char *name,double value);
  ~JsonNode(); 
  JsonNode *AddStringNode(const char *name,const char *value,int index = 0);
  JsonNode *AddIntNode(const char *name,long long value,int index = 0);
  JsonNode *AddDoubleNode(const char *name,double value,int index = 0);
  JsonNode *AddNullNode(const char *name = NULL,int index = 0);
  JsonNode *AddSubNode(JsonNode *subNode ,int index = 0);
  JsonNode *ChildNode();
  JsonNode *PrevNode();
  JsonNode *NextNode();
  JsonNode *FindChildNodeName(const char *name);
  JsonNode *FindSubNodeName(const char *name);
  int DeleteChildNodeName(const char *name);
  int DeleteSubNodeName(const char *name);
  int DeleteNode(JsonNode *subNode);

  int SetName(const char *name);
  int SetStringValue(const char *value);
  int SetIntValue(long long value);
  int SetDoubleValue(double value);
  int SetValueType(NodeType typeV);
  
  int GetName(char *name);
  int GetStringValue(char *value);
  int GetIntValue(long long *value);
  int GetDoubleValue(double *value);
  NodeType GetValueType(void);

  int ToString(char *buff,int size);
  int LoadFromString(const char *buff,int size = 0);
protected:
  int FindAndMallocStr(const char *buff,char **outbuff,int size = 0);
  int FindAndSetNumberStr(const char *buff,int size = 0);
  void InitVariable(void);
private:
  JsonNode *next,*prev,*parent;
  NodeType type;
  NodeValueUnion nodeValue;
  char *namestring;
};


#endif

