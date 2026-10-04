#include <esp_heap_caps.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <mbedtls/platform.h>
#include "tls_pool.h"
// Hidden heap evidence for USB/LAN state only; nothing here is drawn on screen.
// ESP.getMaxAllocHeap() also counts 32-bit-only IRAM, which mbedTLS cannot use,
// so TLS decisions use the byte-addressable internal heap instead.
constexpr uint32_t heap8Caps=MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL;
volatile uint32_t failedAllocs=0,lastFailedAllocSize=0,lastFailedAllocCaps=0;
volatile uint32_t minLargest8=UINT32_MAX,preTlsLargest8=0,preTlsFree8=0,minPreTlsLargest8=UINT32_MAX,failLargest8=0,failFree8=0;
unsigned long lastHeapSample=0;
uint32_t largest8(){return heap_caps_get_largest_free_block(heap8Caps);}
uint32_t free8(){return heap_caps_get_free_size(heap8Caps);}
uint32_t uptimeSeconds(){return (uint32_t)(esp_timer_get_time()/1000000ULL);}
void IRAM_ATTR onAllocFailed(size_t size,uint32_t caps,const char*){failedAllocs++;lastFailedAllocSize=size;lastFailedAllocCaps=caps;}
void noteLargest(uint32_t value){if(value<minLargest8)minLargest8=value;}
void sampleHeap(){if(millis()-lastHeapSample<1000)return;lastHeapSample=millis();noteLargest(largest8());}
// Called by the network worker immediately before each TLS connection.
void notePreTls(){preTlsFree8=free8();preTlsLargest8=largest8();noteLargest(preTlsLargest8);if(preTlsLargest8<minPreTlsLargest8)minPreTlsLargest8=preTlsLargest8;}
void noteTlsFailure(){failFree8=free8();failLargest8=largest8();noteLargest(failLargest8);}
const char* resetReasonName(esp_reset_reason_t r){switch(r){case ESP_RST_POWERON:return "power-on";case ESP_RST_SW:return "software";case ESP_RST_PANIC:return "panic";case ESP_RST_INT_WDT:case ESP_RST_TASK_WDT:case ESP_RST_WDT:return "watchdog";case ESP_RST_BROWNOUT:return "brownout";case ESP_RST_EXT:return "external";case ESP_RST_DEEPSLEEP:return "deep-sleep";default:return "unknown";}}
// TLS record buffers: lent from two blocks reserved once (see tls_pool.h).
// Fallback matches esp_mbedtls_mem_calloc under CONFIG_MBEDTLS_INTERNAL_MEM_ALLOC.
TlsSlabPool tlsPool;
void* tlsCalloc(size_t n,size_t size){void*p=tlsPool.take(n,size);return p?p:heap_caps_calloc(n,size,MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT);}
void tlsFree(void*p){if(!tlsPool.give(p))heap_caps_free(p);}
// Only call while no HTTPS request is running; a failed attempt changes nothing.
bool tlsPoolReserve(){if(tlsPool.ready())return true;auto*a=(uint8_t*)heap_caps_malloc(TlsSlabPool::slabBytes,heap8Caps);auto*b=(uint8_t*)heap_caps_malloc(TlsSlabPool::slabBytes,heap8Caps);if(tlsPool.install(a,b))return true;heap_caps_free(a);heap_caps_free(b);return false;}
volatile int healSimFailures=0; // hidden USB test hook; consumed by the ETA worker
volatile uint32_t etaDocUsage=0;
void heapTelemetryInit(){heap_caps_register_failed_alloc_callback(onAllocFailed);mbedtls_platform_set_calloc_free(tlsCalloc,tlsFree);}
void heapFields(JsonOut&d){
 d.put("uptimeS",uptimeSeconds());d.put("heap8",free8());d.put("largest8",largest8());d.put("minLargest8",minLargest8==UINT32_MAX?0:minLargest8);d.put("minFree8",heap_caps_get_minimum_free_size(heap8Caps));
 d.put("preTlsLargest8",preTlsLargest8);d.put("preTlsFree8",preTlsFree8);d.put("minPreTlsLargest8",minPreTlsLargest8==UINT32_MAX?0:minPreTlsLargest8);d.put("failLargest8",failLargest8);d.put("failFree8",failFree8);
 d.put("tlsSlabs",tlsPool.ready());d.put("tlsSlabHits",tlsPool.hits.load());d.put("tlsSlabMisses",tlsPool.misses.load());d.put("largestIram",heap_caps_get_largest_free_block(MALLOC_CAP_32BIT|MALLOC_CAP_EXEC));
 d.put("etaDocUsage",etaDocUsage);d.put("failedAllocs",failedAllocs);d.put("lastFailedAllocSize",lastFailedAllocSize);d.put("lastFailedAllocCaps",lastFailedAllocCaps);d.put("resetReason",resetReasonName(esp_reset_reason()));
}
// Region-level evidence: heap_caps_print_heap_info writes one line per region.
void emitHeap(){
 heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);multi_heap_info_t info;heap_caps_get_info(&info,heap8Caps);
 Serial.printf("{\"event\":\"heap\",\"uptimeS\":%u,\"free\":%u,\"allocated\":%u,\"largest\":%u,\"minFree\":%u,\"allocatedBlocks\":%u,\"freeBlocks\":%u,\"totalBlocks\":%u,\"internalFree\":%u,\"internalLargest\":%u}\n",uptimeSeconds(),(unsigned)info.total_free_bytes,(unsigned)info.total_allocated_bytes,(unsigned)info.largest_free_block,(unsigned)info.minimum_free_bytes,(unsigned)info.allocated_blocks,(unsigned)info.free_blocks,(unsigned)info.total_blocks,ESP.getFreeHeap(),ESP.getMaxAllocHeap());
}
