diff --git a/component/lwip/api/lwipopts.h b/component/lwip/api/lwipopts.h
index b6d7fdf35..ff2b97675 100644
--- a/component/lwip/api/lwipopts.h
+++ b/component/lwip/api/lwipopts.h
@@ -22,6 +22,7 @@ extern unsigned int sys_now(void);
 #endif
 
 /* ------------------------------------ Common options for all chips ------------------------------------ */
+#define LWIP_PROVIDE_ERRNO 1
 /* Core locking */
 #define LWIP_TCPIP_CORE_LOCKING         1
 /* Memory options */
diff --git a/component/network/CMakeLists.txt b/component/network/CMakeLists.txt
index b4694c105..7f9ce97a7 100644
--- a/component/network/CMakeLists.txt
+++ b/component/network/CMakeLists.txt
@@ -7,7 +7,7 @@ if(CONFIG_CORE_AS_AP)
     ameba_add_subdirectory(dhcp)
     ameba_add_subdirectory(dnrd/src)
     ameba_add_subdirectory(coap)
-    ameba_add_subdirectory(iperf3)
+    # ameba_add_subdirectory(iperf3)
     ameba_add_subdirectory(cJSON)
     ameba_add_subdirectory(httplite)
     ameba_add_subdirectory(iperf)
