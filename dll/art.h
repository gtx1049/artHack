#ifndef ART_H_INCLUDED
#define ART_H_INCLUDED

//using this to align struct
#define PACK __attribute__((gcc_struct, packed))

typedef unsigned short Elf32_Half;
typedef unsigned int Elf32_Word;
typedef unsigned int Elf32_Addr;
typedef unsigned int Elf32_Off;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct oatheader
{
    uint8_t magic_[4];
    uint8_t version_[4];
    uint32_t adler32_checksum_;

    uint32_t instruction_set_;
    uint32_t instruction_set_features_;

    uint32_t dex_file_count_;
    uint32_t executable_offset_;
    uint32_t interpreter_to_interpreter_bridge_offset_;
    uint32_t interpreter_to_compiled_code_bridge_offset_;
    uint32_t jni_dlsym_lookup_offset_;
    uint32_t portable_imt_conflict_trampoline_offset_;
    uint32_t portable_resolution_trampoline_offset_;
    uint32_t portable_to_interpreter_bridge_offset_;
    uint32_t quick_generic_jni_trampoline_offset_;
    uint32_t quick_imt_conflict_trampoline_offset_;
    uint32_t quick_resolution_trampoline_offset_;
    uint32_t quick_to_interpreter_bridge_offset_;

    // The amount that the image this oat is associated with has been patched.
    uint32_t image_patch_delta_;

    uint32_t image_file_location_oat_checksum_;
    uint32_t image_file_location_oat_data_begin_;

    uint32_t key_value_store_size_;

} PACK oatheader;

typedef struct oatmethodheader
{
  // The offset in bytes from the start of the mapping table to the end of the header.
  uint32_t mapping_table_offset_;
  // The offset in bytes from the start of the vmap table to the end of the header.
  uint32_t vmap_table_offset_;
  // The offset in bytes from the start of the gc map to the end of the header.
  uint32_t gc_map_offset_;
  // The stack frame information.
  uint32_t frame_info_frame_size_inbyte;
  uint32_t frame_info_core_spill_mask;
  uint32_t frame_info_fp_spill_mask;
  // The code size in bytes.
  uint32_t code_size_;
} PACK oatmethodheader;

typedef struct oatdexheader
{
    uint32_t dex_file_location_size;
    char* canonical_dex_file_location_;
    uint32_t dex_file_location_checksum_;
    uint32_t dex_file_pointer_;
    uint32_t oat_class_offsets_pointer_;
} PACK oatdexheader;

typedef struct dexheader
{
    uint8_t magic_[8];
    uint32_t checksum_;  // See also location_checksum_
    uint8_t signature_[20];
    uint32_t file_size_;  // size of entire file
    uint32_t header_size_;  // offset to start of next section
    uint32_t endian_tag_;
    uint32_t link_size_;  // unused
    uint32_t link_off_;  // unused
    uint32_t map_off_;  // unused
    uint32_t string_ids_size_;  // number of StringIds
    uint32_t string_ids_off_;  // file offset of StringIds array
    uint32_t type_ids_size_;  // number of TypeIds, we don't support more than 65535
    uint32_t type_ids_off_;  // file offset of TypeIds array
    uint32_t proto_ids_size_;  // number of ProtoIds, we don't support more than 65535
    uint32_t proto_ids_off_;  // file offset of ProtoIds array
    uint32_t field_ids_size_;  // number of FieldIds
    uint32_t field_ids_off_;  // file offset of FieldIds array
    uint32_t method_ids_size_;  // number of MethodIds
    uint32_t method_ids_off_;  // file offset of MethodIds array
    uint32_t class_defs_size_;  // number of ClassDefs
    uint32_t class_defs_off_;  // file offset of ClassDef array
    uint32_t data_size_;  // unused
    uint32_t data_off_;  // unused
} PACK dexheader;

typedef struct classdefitem
{
    uint16_t class_idx_;  // index into type_ids_ array for this class
    uint16_t pad1_;  // padding = 0
    uint32_t access_flags_;
    uint16_t superclass_idx_;  // index into type_ids_ array for superclass
    uint16_t pad2_;  // padding = 0
    uint32_t interfaces_off_;  // file offset to TypeList
    uint32_t source_file_idx_;  // index into string_ids_ for source file name
    uint32_t annotations_off_;  // file offset to annotations_directory_item
    uint32_t class_data_off_;  // file offset to class_data_item
    uint32_t static_values_off_;  // file offset to EncodedArray
} PACK classdefitem;

typedef struct classheader
{

} PACK classheader;

#define EI_NIDENT 16
typedef struct elfheader
{
     unsigned char e_ident[EI_NIDENT];
     Elf32_Half e_type;
     Elf32_Half e_machine;
     Elf32_Word e_version;
     Elf32_Addr e_entry;
     Elf32_Off e_phoff;
     Elf32_Off e_shoff;
     Elf32_Word e_flags;
     Elf32_Half e_ehsize;
     Elf32_Half e_phentsize;
     Elf32_Half e_phnum;
     Elf32_Half e_shentsize;
     Elf32_Half e_shnum;
     Elf32_Half e_shstrndx;
} PACK elfheader;

typedef struct sectionheader
{
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} PACK sectionheader;

typedef struct symentry
{
    Elf32_Word st_name;
    Elf32_Addr st_value;
    Elf32_Word st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half st_shndx;
} PACK symentry;

typedef struct codeitem
{
    unsigned int method_idx_diff;
    unsigned int access_flags;
    unsigned int code_off;
} PACK codeitem;

typedef struct methodiditem
{
    unsigned short class_idx;
    unsigned short proto_idx;
    unsigned int name_idx;
} PACK methodiditem;

typedef struct dexcodeitem
{
    unsigned short registers_size;
    unsigned short ins_size;
    unsigned short outs_size;
    unsigned short tries_size;
    unsigned int debug_info_off;
    unsigned int insns_size;
} PACK dexcodeitem;

typedef struct zipentry
{
    unsigned int frsignature;
    unsigned short frversion;
    unsigned short frflags;
    unsigned short frcompression;
    unsigned short frfiletime;
    unsigned short frfiledate;
    unsigned int frcrc;
    unsigned int frcompressedsize;
    unsigned int fruncompressedsize;
    unsigned short frfilenamelength;
    unsigned short frextrafieldlength;
} PACK zipentry;


#endif // ART_H_INCLUDED
