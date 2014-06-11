#include "streamBuffer.h"

#include <binder/Parcel.h>

#ifdef MODULE_TAG
#undef MODULE_TAG
#endif          
#define MODULE_TAG "magFramework-StreamBuffer"


enum {
    // IStreamBuffer
    SET_USER = IBinder::FIRST_CALL_TRANSACTION,
    SET_BUFFERS,
    ON_BUFFER_EMPTY,
    GET_TYPE,
    RESET,
};

struct BpStreamBuffer : public BpInterface<IStreamBuffer> {
    BpStreamBuffer(const sp<IBinder> &impl)
        : BpInterface<IStreamBuffer>(impl) {
    }

    virtual void setUser(const sp<IStreamBufferUser> &user) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBuffer::getInterfaceDescriptor());
        data.writeStrongBinder(user->asBinder());
        AGILE_LOGD("enter!");
        remote()->transact(SET_USER, data, &reply);
        AGILE_LOGD("exit!");
    }

    virtual void setBuffers(List_t *bufListHead) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBuffer::getInterfaceDescriptor());
        List_t *tmpNode;
        ui32 size = 0;
        BufferNode_t *pBufNode;

        AGILE_LOGD("enter!");
        tmpNode = bufListHead->next;
        while(tmpNode != bufListHead){
            size++;
            tmpNode = tmpNode->next;
        }
        
        data.writeInt32(size);
        tmpNode = bufListHead->next;
        while(tmpNode != bufListHead){
            pBufNode = (BufferNode_t *)list_entry(tmpNode, BufferNode_t, node); 
            data.writeStrongBinder(pBufNode->buffer->asBinder());
            data.writeInt64(pBufNode->pts);
            data.writeInt32(pBufNode->flag);

            tmpNode = tmpNode->next;
        }
        remote()->transact(SET_BUFFERS, data, &reply);
        AGILE_LOGD("exit!");
    }

    virtual void onBufferEmpty(_size_t index, _size_t size) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBuffer::getInterfaceDescriptor());
        data.writeInt32(static_cast<int32_t>(index));
        data.writeInt32(static_cast<int32_t>(size));
        remote()->transact(
                ON_BUFFER_EMPTY, data, &reply /*, IBinder::FLAG_ONEWAY*/);
    }

    virtual Type getType(void) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBuffer::getInterfaceDescriptor());
        remote()->transact(GET_TYPE, data, &reply);
        return static_cast<Type>(reply.readInt32());
    }

    virtual void reset(void) {
        Parcel data, reply;
        data.writeInterfaceToken(IStreamBuffer::getInterfaceDescriptor());
        remote()->transact(
                RESET, data, &reply /*, IBinder::FLAG_ONEWAY*/);
    }
};

IMPLEMENT_META_INTERFACE(StreamBuffer, "android.hardware.IStreamBuffer");

status_t StreamBuffer_Server::onTransact(
        uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags) {
    switch (code) {
        case SET_USER:
        {
            CHECK_INTERFACE(IStreamBuffer, data, reply);
            AGILE_LOGD("SET_USER: enter!");
            setUser(interface_cast<IStreamBufferUser>(data.readStrongBinder()));
            AGILE_LOGD("SET_USER: exit!");
            break;
        }

        case SET_BUFFERS:
        {
            CHECK_INTERFACE(IStreamBuffer, data, reply);
            AGILE_LOGD("SET_BUFFERS: enter!");
            _size_t n = static_cast<_size_t>(data.readInt32());
            List_t *bufHead = (List_t *)mag_mallocz(sizeof(List_t));
            BufferNode_t *p;

            INIT_LIST(bufHead);
            for (_size_t i = 0; i < n; ++i) {
                p = (BufferNode_t *)mag_mallocz(sizeof(BufferNode_t));
                p->buffer = interface_cast<IMemory>(data.readStrongBinder());
                p->pts    = data.readInt64();
                p->flag   = data.readInt32();
                INIT_LIST(&p->node);

                list_add_tail(&p->node, bufHead);
            }
            setBuffers(bufHead);
            AGILE_LOGD("SET_BUFFERS: exit!");
            break;
        }

        case ON_BUFFER_EMPTY:
        {
            CHECK_INTERFACE(IStreamBuffer, data, reply);
            onBufferEmpty(static_cast<_size_t>(data.readInt32()), static_cast<_size_t>(data.readInt32()));
            break;
        }

        case GET_TYPE:
        {
            CHECK_INTERFACE(IStreamBuffer, data, reply);
            reply->writeInt32(static_cast<i32>(getType()));
            break;
        }

        case RESET:
        {
            CHECK_INTERFACE(IStreamBuffer, data, reply);
            reset();
            break;
        }
        
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }

    return OK;
}

