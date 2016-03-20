//
//  variant.cpp
//  GameSrv
//
//  Created by 丁志坚 on 12/16/14.
//
//

#include "msg_variant.h"

//string MsgVariant::toString()
//{
//    string ret;
//    switch (mType) {
//        case kMsgVariantInt:
//            ret = strFormat("%d", *((int*)mValue));
//            break;
//        case kMsgVariantInt64:
//            ret = strFormat("%d", *((int*)mValue));
//            break;
//        case kMsgVariantFloat:
//            ret = strFormat("%f", *((float*)mValue));
//            break;
//        case kMsgVariantString:
//        {
//            string* tmpVal = (string*)mValue;
//            if (tmpVal->empty()) {
//                ret.append("null");
//            } else {
//                ret.append(*tmpVal);
//            }
//            break;
//        }
//        case kMsgVariantObject:
//        {
//            MsgObject* object = ((MsgObject*)mValue);
//            if (object == NULL) {
//                ret.append("object(null)");
//                break;
//            }
//            ret.append("[");
//            int size = object->mArray.size();
//            for (int i = 0; i < size; i++) {
//                if (i > 0) {
//                    ret.append(",");
//                }
//                ret.append((object->mArray)[i].toString());
//            }
//            ret.append("]");
//            break;
//        }
//        case kMsgVariantObjectArray:
//        {
//            DataArray<MsgObject*>* array = (DataArray<MsgObject*>*)mValue;
//            int size = array->size();
//            
//            ret.append("[");
//            for (int i = 0; i < size; i++) {
//                if (i > 0) {
//                    ret.append(",");
//                }
//                ret.append((*array)[i]->toString());
//            }
//            ret.append("]");
//            break;
//        }
//        case kMsgVariantIntArray:
//        {
//            vector<int>* array = (vector<int>*)mValue;
//            ret.append("[");
//            int size = array->size();
//            for (int i = 0; i < size; i++) {
//                if (i > 0) {
//                    ret.append(",");
//                }
//                ret.append(strFormat("%d", (*array)[i]));
//            }
//            ret.append("]");
//            break;
//        }
//        case kMsgVariantInt64Array:
//        {
//            vector<int64_t>* array = (vector<int64_t>*)mValue;
//            ret.append("[");
//            int size = array->size();
//            for (int i = 0; i < size; i++) {
//                if (i > 0) {
//                    ret.append(",");
//                }
//                ret.append(strFormat("%d", (*array)[i]));
//            }
//            ret.append("]");
//            break;
//        }
//        case kMsgVariantFloatArray:
//        {
//            vector<float>* array = (vector<float>*)mValue;
//            ret.append("[");
//            int size = array->size();
//            for (int i = 0; i < size; i++) {
//                if (i > 0) {
//                    ret.append(",");
//                }
//                ret.append(strFormat("%f", (*array)[i]));
//            }
//            ret.append("]");
//            break;
//        }
//        case kMsgVariantStringArray:
//        {
//            vector<string>* array = (vector<string>*)mValue;
//            ret.append("[");
//            int size = array->size();
//            for (int i = 0; i < size; i++) {
//                if (i > 0) {
//                    ret.append(",");
//                }
//                string val = (*array)[i];
//                if (val.empty()) {
//                    val = "null";
//                }
//                ret.append(val);
//            }
//            ret.append("]");
//            break;
//        }
//            
//        default:
//            break;
//    }
//    
//    return ret;
//}
//
//
//
//void MsgVariant::encode(ByteArray& byteArray)
//{
//    
//}
//
//
//bool MsgVariant::decode(ByteArray& byteArray)
//{
//    return true;
//}
//

