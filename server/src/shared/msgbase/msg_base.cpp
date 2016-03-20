#include "msg_base.h"

string MsgObject::toString()
{
    string ret;
    
    ret.append("[");
    for (int i = 0; i < mArray.size(); i++) {
        if (i > 0) {
            ret.append(",");
        }
        string memberStr = variantToString(mArray[i]);
        ret.append(memberStr);
    }
    ret.append("]");
    
    return ret;
}

string MsgObject::variantToString(const MsgVariant &variant)
{
    string ret;
    int variantType = variant.mType;
    void* value = variant.mValue;
    switch (variantType) {
        case kMsgVariantInt:
            ret = strFormat("%d", *(int*)value);
            break;
        case kMsgVariantInt64:
            ret = strFormat("%d", *(int64_t*)value);
            break;
        case kMsgVariantFloat:
            ret = strFormat("%f", *(float*)value);
            break;
        case kMsgVariantString:
        {
            string *tmp = (string*)value;
            if (tmp->empty()) {
                *tmp = "null";
            }
            ret.append(*tmp);
            break;
        }
        case kMsgVariantObject:
        {
            MsgObject* object = (MsgObject*)value;
            ret.append(object->toString());
            break;
        }
        case kMsgVariantObjectArray:
        {
            MsgObjectArray<MsgObject>* array = (MsgObjectArray<MsgObject>*)value;
            int size = array->size();
            ret.append("[");
            for (int i = 0; i < size; i++) {
                if (i > 0) {
                    ret.append(",");
                }
                
                ret.append((*array)[i].toString());
            }
            ret.append("]");
            break;
        }
        case kMsgVariantIntArray:
        {
            vector<int>* array = (vector<int>*)value;
            ret.append("[");
            int size = array->size();
            for (int i = 0; i < size; i++) {
                if (i > 0) {
                    ret.append(",");
                }
                ret.append(strFormat("%d", (*array)[i]));
            }
            ret.append("]");
            break;
        }
        case kMsgVariantInt64Array:
        {
            vector<int64_t>* array = (vector<int64_t>*)value;
            ret.append("[");
            int size = array->size();
            for (int i = 0; i < size; i++) {
                if (i > 0) {
                    ret.append(",");
                }
                ret.append(strFormat("%d", (*array)[i]));
            }
            ret.append("]");
            break;
        }
        case kMsgVariantFloatArray:
        {
            vector<float>* array = (vector<float>*)value;
            ret.append("[");
            int size = array->size();
            for (int i = 0; i < size; i++) {
                if (i > 0) {
                    ret.append(",");
                }
                ret.append(strFormat("%f", (*array)[i]));
            }
            ret.append("]");
            break;
        }
        case kMsgVariantStringArray:
        {
            vector<string>* array = (vector<string>*)value;
            ret.append("[");
            int size = array->size();
            for (int i = 0; i < size; i++) {
                if (i > 0) {
                    ret.append(",");
                }
                string val = (*array)[i];
                if (val.empty()) {
                    val = "null";
                }
                ret.append(val);
            }
            ret.append("]");
            break;
        }
            
        default:
            break;
    }
    
    return ret;
}



void MsgObject::encode(ByteArray& byteArray)
{
    for (int i = 0; i < mArray.size(); i++) {
        encodeVariant(mArray[i], byteArray);
    }
}

void MsgObject::encodeVariant(MsgVariant& variant, ByteArray& byteArray)
{
    int variantType = variant.mType;
    void* value = variant.mValue;
    switch (variantType) {
        case kMsgVariantInt:
            byteArray.write_int(*(int*)value);
            break;
        case kMsgVariantInt64:
            byteArray.write_int64(*(int64_t*)value);
            break;
        case kMsgVariantFloat:
            byteArray.write_float(*(float*)value);
            break;
        case kMsgVariantString:
        {
            string& tmp = *(string*)value;
            byteArray.write_string(tmp);
            break;
        }
        case kMsgVariantObject:
        {
            MsgObject* object = (MsgObject*)value;
            object->encode(byteArray);
            break;
        }
        case kMsgVariantObjectArray:
        {
            MsgObjectArray<MsgObject>* array = (MsgObjectArray<MsgObject>*)value;
            int size = array->size();
            
            byteArray.write_int(size);
            for (int i = 0; i < size; i++) {
                MsgObject* obj = &(*array)[i];
                obj->encode(byteArray);
            }
            break;
        }
        case kMsgVariantIntArray:
        {
            vector<int>* array = (vector<int>*)value;
            int size = array->size();
            byteArray.write_int(size);
            for (int i = 0; i < size; i++) {
                byteArray.write_int((*array)[i]);
            }
            break;
        }
        case kMsgVariantInt64Array:
        {
            vector<int64_t>* array = (vector<int64_t>*)value;
            int size = array->size();
            byteArray.write_int(size);
            for (int i = 0; i < size; i++) {
                byteArray.write_int64((*array)[i]);
            }
            break;
        }
        case kMsgVariantFloatArray:
        {
            vector<float>* array = (vector<float>*)value;
            int size = array->size();
            byteArray.write_int(size);
            for (int i = 0; i < size; i++) {
                byteArray.write_float((*array)[i]);
            }
            break;
        }
        case kMsgVariantStringArray:
        {
            vector<string>* array = (vector<string>*)value;
            int size = array->size();
            byteArray.write_int(size);
            for (int i = 0; i < size; i++) {
                byteArray.write_string((*array)[i]);
            }
            break;
        }
            
        default:
            break;
    }
    

}

void MsgObject::decode(ByteArray& byteArray)
{
    for (int i = 0; i < mArray.size(); i++) {
        decodeVariant(mArray[i], byteArray);
    }
}

void MsgObject::decodeVariant(MsgVariant& variant, ByteArray& byteArray)
{
    int variantType = variant.mType;
    void* value = variant.mValue;
    switch (variantType) {
        case kMsgVariantInt:
            *(int*)value = byteArray.read_int();
            break;
        case kMsgVariantInt64:
            *(int64_t*)value = byteArray.read_int64();
            break;
        case kMsgVariantFloat:
            *(float*)value = byteArray.read_float();
            break;
        case kMsgVariantString:
            *(string*)value = byteArray.read_string();
            break;
        case kMsgVariantObject:
        {
            MsgObject* object = (MsgObject*)value;
            object->decode(byteArray);
            break;
        }
        case kMsgVariantIntArray:
        {
            vector<int>* array = (vector<int>*)value;
            array->clear();
            int size = byteArray.read_int();
            for (int i = 0; i < size; i++) {
                int value = byteArray.read_int();
                array->push_back(value);
            }
            break;
        }
        case kMsgVariantInt64Array:
        {
            vector<int64_t>* array = (vector<int64_t>*)value;
            array->clear();
            int size = byteArray.read_int();
            for (int i = 0; i < size; i++) {
                int64_t value = byteArray.read_int64();
                array->push_back(value);
            }
            break;
        }
        case kMsgVariantFloatArray:
        {
            vector<float>* array = (vector<float>*)value;
            array->clear();
            int size = byteArray.read_int();
            for (int i = 0; i < size; i++) {
                float value = byteArray.read_float();
                array->push_back(value);
            }
            break;
        }
        case kMsgVariantStringArray:
        {
            vector<string>* array = (vector<string>*)value;
            array->clear();
            int size = byteArray.read_int();
            for (int i = 0; i < size; i++) {
                string str = byteArray.read_string();
                array->push_back(str);
            }
            break;
        }
        case kMsgVariantObjectArray:
        {
            MsgObject* prototype = variant.mPrototype;
            int size = byteArray.read_int();
            MsgObjectArray<MsgObject>* array = (MsgObjectArray<MsgObject>*)value;
            array->clear();
            for (int i = 0; i < size; i++) {
                prototype->decode(byteArray);
                array->push_back(*prototype);
            }
            break;
        }
            
        default:
            break;
    }
}
