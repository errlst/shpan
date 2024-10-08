// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: entry.proto

#include "entry.pb.h"

#include <algorithm>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>

PROTOBUF_PRAGMA_INIT_SEG

namespace _pb = ::PROTOBUF_NAMESPACE_ID;
namespace _pbi = _pb::internal;

namespace proto {
PROTOBUF_CONSTEXPR EntryItem::EntryItem(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.path_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.shared_link_)*/{&::_pbi::fixed_address_empty_string, ::_pbi::ConstantInitialized{}}
  , /*decltype(_impl_.size_)*/uint64_t{0u}
  , /*decltype(_impl_.is_dir_)*/false
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct EntryItemDefaultTypeInternal {
  PROTOBUF_CONSTEXPR EntryItemDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~EntryItemDefaultTypeInternal() {}
  union {
    EntryItem _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 EntryItemDefaultTypeInternal _EntryItem_default_instance_;
PROTOBUF_CONSTEXPR Entrys::Entrys(
    ::_pbi::ConstantInitialized): _impl_{
    /*decltype(_impl_.entrys_)*/{}
  , /*decltype(_impl_._cached_size_)*/{}} {}
struct EntrysDefaultTypeInternal {
  PROTOBUF_CONSTEXPR EntrysDefaultTypeInternal()
      : _instance(::_pbi::ConstantInitialized{}) {}
  ~EntrysDefaultTypeInternal() {}
  union {
    Entrys _instance;
  };
};
PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 EntrysDefaultTypeInternal _Entrys_default_instance_;
}  // namespace proto
static ::_pb::Metadata file_level_metadata_entry_2eproto[2];
static constexpr ::_pb::EnumDescriptor const** file_level_enum_descriptors_entry_2eproto = nullptr;
static constexpr ::_pb::ServiceDescriptor const** file_level_service_descriptors_entry_2eproto = nullptr;

const uint32_t TableStruct_entry_2eproto::offsets[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::proto::EntryItem, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::proto::EntryItem, _impl_.size_),
  PROTOBUF_FIELD_OFFSET(::proto::EntryItem, _impl_.is_dir_),
  PROTOBUF_FIELD_OFFSET(::proto::EntryItem, _impl_.path_),
  PROTOBUF_FIELD_OFFSET(::proto::EntryItem, _impl_.shared_link_),
  ~0u,  // no _has_bits_
  PROTOBUF_FIELD_OFFSET(::proto::Entrys, _internal_metadata_),
  ~0u,  // no _extensions_
  ~0u,  // no _oneof_case_
  ~0u,  // no _weak_field_map_
  ~0u,  // no _inlined_string_donated_
  PROTOBUF_FIELD_OFFSET(::proto::Entrys, _impl_.entrys_),
};
static const ::_pbi::MigrationSchema schemas[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
  { 0, -1, -1, sizeof(::proto::EntryItem)},
  { 10, -1, -1, sizeof(::proto::Entrys)},
};

static const ::_pb::Message* const file_default_instances[] = {
  &::proto::_EntryItem_default_instance_._instance,
  &::proto::_Entrys_default_instance_._instance,
};

const char descriptor_table_protodef_entry_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) =
  "\n\013entry.proto\022\005proto\"L\n\tEntryItem\022\014\n\004siz"
  "e\030\001 \001(\004\022\016\n\006is_dir\030\002 \001(\010\022\014\n\004path\030\003 \001(\t\022\023\n"
  "\013shared_link\030\004 \001(\t\"*\n\006Entrys\022 \n\006entrys\030\001"
  " \003(\0132\020.proto.EntryItemb\006proto3"
  ;
static ::_pbi::once_flag descriptor_table_entry_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_entry_2eproto = {
    false, false, 150, descriptor_table_protodef_entry_2eproto,
    "entry.proto",
    &descriptor_table_entry_2eproto_once, nullptr, 0, 2,
    schemas, file_default_instances, TableStruct_entry_2eproto::offsets,
    file_level_metadata_entry_2eproto, file_level_enum_descriptors_entry_2eproto,
    file_level_service_descriptors_entry_2eproto,
};
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_entry_2eproto_getter() {
  return &descriptor_table_entry_2eproto;
}

// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2 static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_entry_2eproto(&descriptor_table_entry_2eproto);
namespace proto {

// ===================================================================

class EntryItem::_Internal {
 public:
};

EntryItem::EntryItem(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:proto.EntryItem)
}
EntryItem::EntryItem(const EntryItem& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  EntryItem* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.path_){}
    , decltype(_impl_.shared_link_){}
    , decltype(_impl_.size_){}
    , decltype(_impl_.is_dir_){}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  _impl_.path_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.path_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_path().empty()) {
    _this->_impl_.path_.Set(from._internal_path(), 
      _this->GetArenaForAllocation());
  }
  _impl_.shared_link_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.shared_link_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (!from._internal_shared_link().empty()) {
    _this->_impl_.shared_link_.Set(from._internal_shared_link(), 
      _this->GetArenaForAllocation());
  }
  ::memcpy(&_impl_.size_, &from._impl_.size_,
    static_cast<size_t>(reinterpret_cast<char*>(&_impl_.is_dir_) -
    reinterpret_cast<char*>(&_impl_.size_)) + sizeof(_impl_.is_dir_));
  // @@protoc_insertion_point(copy_constructor:proto.EntryItem)
}

inline void EntryItem::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.path_){}
    , decltype(_impl_.shared_link_){}
    , decltype(_impl_.size_){uint64_t{0u}}
    , decltype(_impl_.is_dir_){false}
    , /*decltype(_impl_._cached_size_)*/{}
  };
  _impl_.path_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.path_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  _impl_.shared_link_.InitDefault();
  #ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
    _impl_.shared_link_.Set("", GetArenaForAllocation());
  #endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
}

EntryItem::~EntryItem() {
  // @@protoc_insertion_point(destructor:proto.EntryItem)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void EntryItem::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.path_.Destroy();
  _impl_.shared_link_.Destroy();
}

void EntryItem::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void EntryItem::Clear() {
// @@protoc_insertion_point(message_clear_start:proto.EntryItem)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.path_.ClearToEmpty();
  _impl_.shared_link_.ClearToEmpty();
  ::memset(&_impl_.size_, 0, static_cast<size_t>(
      reinterpret_cast<char*>(&_impl_.is_dir_) -
      reinterpret_cast<char*>(&_impl_.size_)) + sizeof(_impl_.is_dir_));
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* EntryItem::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // uint64 size = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 8)) {
          _impl_.size_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // bool is_dir = 2;
      case 2:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 16)) {
          _impl_.is_dir_ = ::PROTOBUF_NAMESPACE_ID::internal::ReadVarint64(&ptr);
          CHK_(ptr);
        } else
          goto handle_unusual;
        continue;
      // string path = 3;
      case 3:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 26)) {
          auto str = _internal_mutable_path();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "proto.EntryItem.path"));
        } else
          goto handle_unusual;
        continue;
      // string shared_link = 4;
      case 4:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 34)) {
          auto str = _internal_mutable_shared_link();
          ptr = ::_pbi::InlineGreedyStringParser(str, ptr, ctx);
          CHK_(ptr);
          CHK_(::_pbi::VerifyUTF8(str, "proto.EntryItem.shared_link"));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* EntryItem::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:proto.EntryItem)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // uint64 size = 1;
  if (this->_internal_size() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteUInt64ToArray(1, this->_internal_size(), target);
  }

  // bool is_dir = 2;
  if (this->_internal_is_dir() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteBoolToArray(2, this->_internal_is_dir(), target);
  }

  // string path = 3;
  if (!this->_internal_path().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_path().data(), static_cast<int>(this->_internal_path().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "proto.EntryItem.path");
    target = stream->WriteStringMaybeAliased(
        3, this->_internal_path(), target);
  }

  // string shared_link = 4;
  if (!this->_internal_shared_link().empty()) {
    ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::VerifyUtf8String(
      this->_internal_shared_link().data(), static_cast<int>(this->_internal_shared_link().length()),
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::SERIALIZE,
      "proto.EntryItem.shared_link");
    target = stream->WriteStringMaybeAliased(
        4, this->_internal_shared_link(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:proto.EntryItem)
  return target;
}

size_t EntryItem::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:proto.EntryItem)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string path = 3;
  if (!this->_internal_path().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_path());
  }

  // string shared_link = 4;
  if (!this->_internal_shared_link().empty()) {
    total_size += 1 +
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::StringSize(
        this->_internal_shared_link());
  }

  // uint64 size = 1;
  if (this->_internal_size() != 0) {
    total_size += ::_pbi::WireFormatLite::UInt64SizePlusOne(this->_internal_size());
  }

  // bool is_dir = 2;
  if (this->_internal_is_dir() != 0) {
    total_size += 1 + 1;
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData EntryItem::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    EntryItem::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*EntryItem::GetClassData() const { return &_class_data_; }


void EntryItem::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<EntryItem*>(&to_msg);
  auto& from = static_cast<const EntryItem&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:proto.EntryItem)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_path().empty()) {
    _this->_internal_set_path(from._internal_path());
  }
  if (!from._internal_shared_link().empty()) {
    _this->_internal_set_shared_link(from._internal_shared_link());
  }
  if (from._internal_size() != 0) {
    _this->_internal_set_size(from._internal_size());
  }
  if (from._internal_is_dir() != 0) {
    _this->_internal_set_is_dir(from._internal_is_dir());
  }
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void EntryItem::CopyFrom(const EntryItem& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:proto.EntryItem)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool EntryItem::IsInitialized() const {
  return true;
}

void EntryItem::InternalSwap(EntryItem* other) {
  using std::swap;
  auto* lhs_arena = GetArenaForAllocation();
  auto* rhs_arena = other->GetArenaForAllocation();
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.path_, lhs_arena,
      &other->_impl_.path_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr::InternalSwap(
      &_impl_.shared_link_, lhs_arena,
      &other->_impl_.shared_link_, rhs_arena
  );
  ::PROTOBUF_NAMESPACE_ID::internal::memswap<
      PROTOBUF_FIELD_OFFSET(EntryItem, _impl_.is_dir_)
      + sizeof(EntryItem::_impl_.is_dir_)
      - PROTOBUF_FIELD_OFFSET(EntryItem, _impl_.size_)>(
          reinterpret_cast<char*>(&_impl_.size_),
          reinterpret_cast<char*>(&other->_impl_.size_));
}

::PROTOBUF_NAMESPACE_ID::Metadata EntryItem::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_entry_2eproto_getter, &descriptor_table_entry_2eproto_once,
      file_level_metadata_entry_2eproto[0]);
}

// ===================================================================

class Entrys::_Internal {
 public:
};

Entrys::Entrys(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                         bool is_message_owned)
  : ::PROTOBUF_NAMESPACE_ID::Message(arena, is_message_owned) {
  SharedCtor(arena, is_message_owned);
  // @@protoc_insertion_point(arena_constructor:proto.Entrys)
}
Entrys::Entrys(const Entrys& from)
  : ::PROTOBUF_NAMESPACE_ID::Message() {
  Entrys* const _this = this; (void)_this;
  new (&_impl_) Impl_{
      decltype(_impl_.entrys_){from._impl_.entrys_}
    , /*decltype(_impl_._cached_size_)*/{}};

  _internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
  // @@protoc_insertion_point(copy_constructor:proto.Entrys)
}

inline void Entrys::SharedCtor(
    ::_pb::Arena* arena, bool is_message_owned) {
  (void)arena;
  (void)is_message_owned;
  new (&_impl_) Impl_{
      decltype(_impl_.entrys_){arena}
    , /*decltype(_impl_._cached_size_)*/{}
  };
}

Entrys::~Entrys() {
  // @@protoc_insertion_point(destructor:proto.Entrys)
  if (auto *arena = _internal_metadata_.DeleteReturnArena<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>()) {
  (void)arena;
    return;
  }
  SharedDtor();
}

inline void Entrys::SharedDtor() {
  GOOGLE_DCHECK(GetArenaForAllocation() == nullptr);
  _impl_.entrys_.~RepeatedPtrField();
}

void Entrys::SetCachedSize(int size) const {
  _impl_._cached_size_.Set(size);
}

void Entrys::Clear() {
// @@protoc_insertion_point(message_clear_start:proto.Entrys)
  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.entrys_.Clear();
  _internal_metadata_.Clear<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>();
}

const char* Entrys::_InternalParse(const char* ptr, ::_pbi::ParseContext* ctx) {
#define CHK_(x) if (PROTOBUF_PREDICT_FALSE(!(x))) goto failure
  while (!ctx->Done(&ptr)) {
    uint32_t tag;
    ptr = ::_pbi::ReadTag(ptr, &tag);
    switch (tag >> 3) {
      // repeated .proto.EntryItem entrys = 1;
      case 1:
        if (PROTOBUF_PREDICT_TRUE(static_cast<uint8_t>(tag) == 10)) {
          ptr -= 1;
          do {
            ptr += 1;
            ptr = ctx->ParseMessage(_internal_add_entrys(), ptr);
            CHK_(ptr);
            if (!ctx->DataAvailable(ptr)) break;
          } while (::PROTOBUF_NAMESPACE_ID::internal::ExpectTag<10>(ptr));
        } else
          goto handle_unusual;
        continue;
      default:
        goto handle_unusual;
    }  // switch
  handle_unusual:
    if ((tag == 0) || ((tag & 7) == 4)) {
      CHK_(ptr);
      ctx->SetLastTag(tag);
      goto message_done;
    }
    ptr = UnknownFieldParse(
        tag,
        _internal_metadata_.mutable_unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(),
        ptr, ctx);
    CHK_(ptr != nullptr);
  }  // while
message_done:
  return ptr;
failure:
  ptr = nullptr;
  goto message_done;
#undef CHK_
}

uint8_t* Entrys::_InternalSerialize(
    uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:proto.Entrys)
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  // repeated .proto.EntryItem entrys = 1;
  for (unsigned i = 0,
      n = static_cast<unsigned>(this->_internal_entrys_size()); i < n; i++) {
    const auto& repfield = this->_internal_entrys(i);
    target = ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::
        InternalWriteMessage(1, repfield, repfield.GetCachedSize(), target, stream);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target = ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
        _internal_metadata_.unknown_fields<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(::PROTOBUF_NAMESPACE_ID::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:proto.Entrys)
  return target;
}

size_t Entrys::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:proto.Entrys)
  size_t total_size = 0;

  uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // repeated .proto.EntryItem entrys = 1;
  total_size += 1UL * this->_internal_entrys_size();
  for (const auto& msg : this->_impl_.entrys_) {
    total_size +=
      ::PROTOBUF_NAMESPACE_ID::internal::WireFormatLite::MessageSize(msg);
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}

const ::PROTOBUF_NAMESPACE_ID::Message::ClassData Entrys::_class_data_ = {
    ::PROTOBUF_NAMESPACE_ID::Message::CopyWithSourceCheck,
    Entrys::MergeImpl
};
const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*Entrys::GetClassData() const { return &_class_data_; }


void Entrys::MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg) {
  auto* const _this = static_cast<Entrys*>(&to_msg);
  auto& from = static_cast<const Entrys&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:proto.Entrys)
  GOOGLE_DCHECK_NE(&from, _this);
  uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  _this->_impl_.entrys_.MergeFrom(from._impl_.entrys_);
  _this->_internal_metadata_.MergeFrom<::PROTOBUF_NAMESPACE_ID::UnknownFieldSet>(from._internal_metadata_);
}

void Entrys::CopyFrom(const Entrys& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:proto.Entrys)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Entrys::IsInitialized() const {
  return true;
}

void Entrys::InternalSwap(Entrys* other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  _impl_.entrys_.InternalSwap(&other->_impl_.entrys_);
}

::PROTOBUF_NAMESPACE_ID::Metadata Entrys::GetMetadata() const {
  return ::_pbi::AssignDescriptors(
      &descriptor_table_entry_2eproto_getter, &descriptor_table_entry_2eproto_once,
      file_level_metadata_entry_2eproto[1]);
}

// @@protoc_insertion_point(namespace_scope)
}  // namespace proto
PROTOBUF_NAMESPACE_OPEN
template<> PROTOBUF_NOINLINE ::proto::EntryItem*
Arena::CreateMaybeMessage< ::proto::EntryItem >(Arena* arena) {
  return Arena::CreateMessageInternal< ::proto::EntryItem >(arena);
}
template<> PROTOBUF_NOINLINE ::proto::Entrys*
Arena::CreateMaybeMessage< ::proto::Entrys >(Arena* arena) {
  return Arena::CreateMessageInternal< ::proto::Entrys >(arena);
}
PROTOBUF_NAMESPACE_CLOSE

// @@protoc_insertion_point(global_scope)
#include <google/protobuf/port_undef.inc>
