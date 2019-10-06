/**
 * Copyright 2019 Aaron Robert
 * */
#include "rpc/rpc.h"

#pragma region ServerProxy
Data* ServeHandle(void* context, Data* const request) { return nullptr; }

void ServerProxy::Serve() { this->channel_->Serve(this, ServeHandle); }
#pragma endregion
