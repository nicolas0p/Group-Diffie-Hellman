// EPOS PC Run-Time System Information Implementation

#include <utility/debug.h>
#include <machine/pc/info.h>

__BEGIN_SYS

Debug & operator<<(Debug & db, const System_Info & si)
{
    db << "{"
       << "\nBoot_Map={"
       << "n_cpus=" << si.bm.n_cpus
       << ",mem_base=" << reinterpret_cast<void *>(si.bm.mem_base)
       << ",mem_top=" << reinterpret_cast<void *>(si.bm.mem_top)
       << ",io_base=" << reinterpret_cast<void *>(si.bm.io_base)
       << ",io_top=" << reinterpret_cast<void *>(si.bm.io_top)
       << ",node_id=" << si.bm.node_id
       << ",img_size=" << si.bm.img_size
       << ",setup_offset=" << hex << si.bm.setup_offset
       << ",init_offset=" << si.bm.init_offset
       << ",system_offset=" << si.bm.system_offset
       << ",application_offset=" << si.bm.application_offset
       << ",extras_offset=" << si.bm.extras_offset << dec
       << "}"
       << "\nPhysical_Memory_Map={"
       << "mem_base=" << reinterpret_cast<void *>(si.pmm.mem_base)
       << ",mem_top=" << reinterpret_cast<void *>(si.pmm.mem_top)
       << ",io_base=" << reinterpret_cast<void *>(si.pmm.io_base)
       << ",io_top=" << reinterpret_cast<void *>(si.pmm.io_top)
       << ",ext_base=" << reinterpret_cast<void *>(si.pmm.ext_base)
       << ",ext_top=" << reinterpret_cast<void *>(si.pmm.ext_top)
       << ",idt=" << reinterpret_cast<void *>(si.pmm.idt)
       << ",gdt=" << reinterpret_cast<void *>(si.pmm.gdt)
       << ",sys_pt=" << reinterpret_cast<void *>(si.pmm.sys_pt)
       << ",sys_pd=" << reinterpret_cast<void *>(si.pmm.sys_pd)
       << ",sys_info=" << reinterpret_cast<void *>(si.pmm.sys_info)
       << ",phy_mem_pts=" << reinterpret_cast<void *>(si.pmm.phy_mem_pts)
       << ",io_pts=" << reinterpret_cast<void *>(si.pmm.io_pts)
       << ",sys_code=" << reinterpret_cast<void *>(si.pmm.sys_code)
       << ",sys_data=" << reinterpret_cast<void *>(si.pmm.sys_data)
       << ",sys_stack=" << reinterpret_cast<void *>(si.pmm.sys_stack)
       << ",free1_base=" << reinterpret_cast<void *>(si.pmm.free1_base)
       << ",free1_top=" << reinterpret_cast<void *>(si.pmm.free1_top)
       << ",free2_base=" << reinterpret_cast<void *>(si.pmm.free2_base)
       << ",free2_top=" << reinterpret_cast<void *>(si.pmm.free2_top)
       << ",free3_base=" << reinterpret_cast<void *>(si.pmm.free3_base)
       << ",free3_top=" << reinterpret_cast<void *>(si.pmm.free3_top)
       << "}"
       << "\nLoad_Map={"
       << "has_stp=" << si.lm.has_stp
       << ",has_ini=" << si.lm.has_ini
       << ",has_sys=" << si.lm.has_sys
       << ",has_app=" << si.lm.has_app
       << ",has_ext=" << si.lm.has_ext
       << ",stp_entry=" << reinterpret_cast<void *>(si.lm.stp_entry)
       << ",stp_segments=" << si.lm.stp_segments
       << ",stp_code=" << reinterpret_cast<void *>(si.lm.stp_code)
       << ",stp_code_size=" << si.lm.stp_code_size
       << ",stp_data=" << reinterpret_cast<void *>(si.lm.stp_data)
       << ",stp_data_size=" << si.lm.stp_data_size
       << ",ini_entry=" << reinterpret_cast<void *>(si.lm.ini_entry)
       << ",ini_segments=" << si.lm.ini_segments
       << ",ini_code=" << reinterpret_cast<void *>(si.lm.ini_code)
       << ",ini_code_size=" << si.lm.ini_code_size
       << ",ini_data=" << reinterpret_cast<void *>(si.lm.ini_data)
       << ",ini_data_size=" << si.lm.ini_data_size
       << ",sys_entry=" << reinterpret_cast<void *>(si.lm.sys_entry)
       << ",sys_segments=" << si.lm.sys_segments
       << ",sys_code=" << reinterpret_cast<void *>(si.lm.sys_code)
       << ",sys_code_size=" << si.lm.sys_code_size
       << ",sys_data=" << reinterpret_cast<void *>(si.lm.sys_data)
       << ",sys_data_size=" << si.lm.sys_data_size
       << ",sys_stack=" << reinterpret_cast<void *>(si.lm.sys_stack)
       << ",sys_stack_size=" << si.lm.sys_stack_size
       << ",app_entry=" << reinterpret_cast<void *>(si.lm.app_entry)
       << ",app_segments=" << si.lm.app_segments
       << ",app_code=" << reinterpret_cast<void *>(si.lm.app_code)
       << ",app_code_size=" << si.lm.app_code_size
       << ",app_data=" << reinterpret_cast<void *>(si.lm.app_data)
       << ",app_stack=" << reinterpret_cast<void *>(si.lm.app_stack)
       << ",app_heap=" << reinterpret_cast<void *>(si.lm.app_heap)
       << ",app_data_size=" << si.lm.app_data_size
       << ",app_extra=" << reinterpret_cast<void *>(si.lm.app_extra)
       << ",app_extra_size=" << si.lm.app_extra_size
       << "}"
       << "\nTime_Map={"
       << "cpu_clock=" << si.tm.cpu_clock
       << ",bus_clock=" << si.tm.bus_clock
       << "}"
       << "}";

    return db;
}

__END_SYS
