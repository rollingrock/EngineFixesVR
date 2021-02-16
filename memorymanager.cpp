#include "version.h"


#include "REL/Relocation.h"
#include "SKSE/API.h"
#include "SKSE/CodeGenerator.h"
#include "SKSE/SafeWrite.h"
#include "SKSE/Trampoline.h"

#include "RE/MemoryManager.h"

#include "tbb/scalable_allocator.h"

#include "xbyak/xbyak.h"    

namespace RE
{
    struct BSThreadEvent
    {
        static void InitSDM()
        {
            using func_t = decltype(&BSThreadEvent::InitSDM);
          //  REL::Relocation<func_t> func{ REL::ID(67151) };
            REL::Offset<func_t> func(0xc485e0);
            return func();
        }
    };
}


namespace
{
    std::byte* g_trash{ nullptr };  // a dumb workaround for reads from/writes to zero size allocations

    namespace detail
    {
        struct asm_patch :
            Xbyak::CodeGenerator
        {
            asm_patch(std::uintptr_t a_dst)
            {
                mov(rax, a_dst);
                jmp(rax);
            }
        };
    }

    void asm_jump(std::uintptr_t a_from, [[maybe_unused]] std::size_t a_size, std::uintptr_t a_to)
    {
        detail::asm_patch p{ a_to };
        p.ready();
        assert(p.getSize() <= a_size);
        //REL::safe_write(
        //    a_from,
        //    std::span{ p.getCode<const std::byte*>(), p.getSize() });

        SKSE::SafeWriteBuf(a_from, p.getCode<const std::byte*>(), p.getSize());
    }

    namespace AutoScrapBuffer
    {
        void Ctor()
        {
            //REL::Relocation<std::uintptr_t> target{ REL::ID(66853), 0x1D };
            REL::Offset<std::uintptr_t> target = 0xc3cd90 + 0x1D;
            constexpr std::size_t size = 0x32 - 0x1D;
            //REL::safe_fill(target.address(), REL::NOP, size);

            for (int i = 0; i < size; ++i)
            {
                SKSE::SafeWrite8(target.GetAddress() + i, 0x90);
            }
        }

        void Dtor()
        {
//            REL::Relocation<std::uintptr_t> base{ REL::ID(66854) };
            REL::Offset<std::uintptr_t> base = 0xc3ce20;

            {
                struct Patch :
                    Xbyak::CodeGenerator
                {
                    Patch()
                    {
                        xor_(rax, rax);
                        cmp(rbx, rax);
                    }
                };

                const auto dst = base.GetAddress() + 0x12;
                constexpr std::size_t size = 0x2F - 0x12;
                for (int i = 0; i < size; ++i)
                {
                    SKSE::SafeWrite8(dst + i, 0x90);
                }
                //REL::safe_fill(dst, REL::NOP, size);

                Patch p;
                p.ready();
                assert(p.getSize() <= size);
                //REL::safe_write(
                //    dst,
                //    stl::span{ p.getCode<const std::byte*>(), p.getSize() });

                SKSE::SafeWriteBuf(dst, p.getCode<const std::byte*>(), p.getSize());
            }

            {
                const auto dst = base.GetAddress() + 0x2F;
              //  REL::safe_write(dst, std::uint8_t{ 0x74 });  // jnz -> jz
                SKSE::SafeWrite8(dst, 0x74);
            }
        }

        void Install()
        {
            Ctor();
            Dtor();
        }
    }

    namespace MemoryManager
    {
        void* Allocate(RE::MemoryManager*, std::size_t a_size, std::uint32_t a_alignment, bool a_alignmentRequired)
        {
            if (a_size > 0)
                return a_alignmentRequired ?
                scalable_aligned_malloc(a_size, a_alignment) :
                scalable_malloc(a_size);
            else
                return g_trash;
        }

        void Deallocate(RE::MemoryManager*, void* a_mem, bool a_alignmentRequired)
        {
            if (a_mem != g_trash)
                a_alignmentRequired ?
                scalable_aligned_free(a_mem) :
                scalable_free(a_mem);
        }

        void* Reallocate(RE::MemoryManager* a_self, void* a_oldMem, std::size_t a_newSize, std::uint32_t a_alignment, bool a_alignmentRequired)
        {
            if (a_oldMem == g_trash)
                return Allocate(a_self, a_newSize, a_alignment, a_alignmentRequired);
            else
                return a_alignmentRequired ?
                scalable_aligned_realloc(a_oldMem, a_newSize, a_alignment) :
                scalable_realloc(a_oldMem, a_newSize);
        }

        void ReplaceAllocRoutines()
        {
            using tuple_t = std::tuple<std::uint64_t, std::size_t, void*>;
            const std::array todo{
                //tuple_t{ 66859, 0x248, &Allocate },
                //tuple_t{ 66861, 0x114, &Deallocate },
                //tuple_t{ 66860, 0xA7, &Reallocate },
                tuple_t{ 0xc3d0e0, 0x248, &Allocate },
                tuple_t{ 0xc3d3e0, 0x114, &Deallocate },
                tuple_t{ 0xc3d330, 0xA7, &Reallocate },
            };

            for (const auto& [id, size, func] : todo)
            {
               // REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
                REL::Offset<std::uintptr_t> target = id;
                
                for (int i = 0; i < size; ++i)
                {
                    SKSE::SafeWrite8(target.GetAddress() + i, 0xCC);
                }
                //REL::safe_fill(target.address(), REL::INT3, size);
                asm_jump(target.GetAddress(), size, reinterpret_cast<std::uintptr_t>(func));
            }
        }

        void StubInit()
        {
           // REL::Relocation<std::uintptr_t> target{ REL::ID(66862) };
            REL::Offset<std::uintptr_t> target = 0xc3d550;

          //  REL::safe_fill(target.address(), REL::INT3, 0x9E);
            for (int i = 0; i < 0x9E; ++i)
            {
                SKSE::SafeWrite8(target.GetAddress() + i, 0xCC);
            }

           // REL::safe_write(target.address(), REL::RET);
            SKSE::SafeWrite8(target.GetAddress(), 0xC3);
        }

        void Install()
        {
            StubInit();
            ReplaceAllocRoutines();
            RE::MemoryManager::GetSingleton()->RegisterMemoryManager();
            RE::BSThreadEvent::InitSDM();
        }
    }

    namespace ScrapHeap
    {
        void* Allocate(RE::ScrapHeap*, std::size_t a_size, std::size_t a_alignment)
        {
            return a_size > 0 ?
                scalable_aligned_malloc(a_size, a_alignment) :
                g_trash;
        }

        RE::ScrapHeap* Ctor(RE::ScrapHeap* a_this)
        {
            std::memset(a_this, 0, sizeof(RE::ScrapHeap));
            //reinterpret_cast<std::uintptr_t*>(a_this)[0] = REL::ID(285161).address();
            reinterpret_cast<std::uintptr_t*>(a_this)[0] = 0x17e4168;
            return a_this;
        }

        void Deallocate(RE::ScrapHeap*, void* a_mem)
        {
            if (a_mem != g_trash)
                scalable_aligned_free(a_mem);
        }

        void WriteHooks()
        {
            using tuple_t = std::tuple<std::uint64_t, std::size_t, void*>;
            const std::array todo{
                //tuple_t{ 66884, 0x607, &Allocate },
                //tuple_t{ 66885, 0x143, &Deallocate },
                //tuple_t{ 66882, 0x12B, &Ctor },
                tuple_t{ 0xc3e320, 0x607, &Allocate },
                tuple_t{ 0xc3e940, 0x143, &Deallocate },
                tuple_t{ 0xc3e1a0, 0x12B, &Ctor },

            };

            for (const auto& [id, size, func] : todo)
            {
                //REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
                REL::Offset<std::uintptr_t> target = id;

                //REL::safe_fill(target.address(), REL::INT3, size);
                for (int i = 0; i < size; ++i)
                {
                    SKSE::SafeWrite8(target.GetAddress() + i, 0xCC);
                }
                asm_jump(target.GetAddress(), size, reinterpret_cast<std::uintptr_t>(func));
            }
        }

        void WriteStubs()
        {
            using tuple_t = std::tuple<std::uint64_t, std::size_t>;
            const std::array todo{
                //tuple_t{ 66891, 0xC3 },   // Clean
                //tuple_t{ 66890, 0x8 },    // ClearKeepPages
                //tuple_t{ 66894, 0xF6 },   // InsertFreeBlock
                //tuple_t{ 66895, 0x183 },  // RemoveFreeBlock
                //tuple_t{ 66889, 0x4 },    // SetKeepPages
                //tuple_t{ 66883, 0x32 },   // dtor
                tuple_t{ 0xc3eb00, 0xC3 },   // Clean
                tuple_t{ 0xc3eaf0, 0x8 },    // ClearKeepPages
                tuple_t{ 0xc3ec30, 0xF6 },   // InsertFreeBlock
                tuple_t{ 0xc3ed30, 0x183 },  // RemoveFreeBlock
                tuple_t{ 0xc3eaf0, 0x4 },    // SetKeepPages
                tuple_t{ 0xc3e2e0, 0x32 },   // dtor
            };

            for (const auto& [id, size] : todo)
            {
              //  REL::Relocation<std::uintptr_t> target{ REL::ID(id) };
                REL::Offset<std::uintptr_t> target = id;
             //   REL::safe_fill(target.address(), REL::INT3, size);
                for (int i = 0; i < size; ++i)
                {
                    SKSE::SafeWrite8(target.GetAddress() + i, 0xCC);
                }
                //REL::safe_write(target.address(), REL::RET);
                SKSE::SafeWrite8(target.GetAddress(), 0xC3);
            }
        }

        void Install()
        {
            WriteStubs();
            WriteHooks();
        }
    }
}

namespace patches
{
    bool PatchMemoryManager()
    {
        _MESSAGE("-memory manager patch -");

        g_trash = new std::byte[1u << 10]{ static_cast<std::byte>(0) };

        AutoScrapBuffer::Install();
        MemoryManager::Install();
        ScrapHeap::Install();

        _MESSAGE("success");
        return true;
    }
}
