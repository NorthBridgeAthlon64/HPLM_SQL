cmake_minimum_required(VERSION 3.20)
project(HPLM VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找 Qt6 模块
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Charts Network)

# 源代码
set(SOURCES
    src/main.cpp

    # db 层
    src/db/DatabaseManager.cpp

    # repository 层（18 个 Repo）
    src/db/repository/SupplierRepo.cpp
    src/db/repository/ComponentRepo.cpp
    src/db/repository/ComponentPriceRepo.cpp
    src/db/repository/EmployeeRepo.cpp
    src/db/repository/ProductRepo.cpp
    src/db/repository/ProductVersionRepo.cpp
    src/db/repository/VersionCostRepo.cpp
    src/db/repository/ProductBOMRepo.cpp
    src/db/repository/ProductionBatchRepo.cpp
    src/db/repository/BatchMaterialRepo.cpp
    src/db/repository/TestRecordRepo.cpp
    src/db/repository/CustomerRepo.cpp
    src/db/repository/RepairRecordRepo.cpp
    src/db/repository/RepairMaterialRepo.cpp
    src/db/repository/WarehouseRepo.cpp
    src/db/repository/InventoryRepo.cpp
    src/db/repository/InventoryTransactionRepo.cpp
    src/db/repository/ProductFeedbackRepo.cpp

    # service 层（6 个 Service）
    src/service/AuthService.cpp
    src/service/InventoryService.cpp
    src/service/ProductionService.cpp
    src/service/RepairService.cpp
    src/service/TraceService.cpp
    src/service/FeedbackService.cpp

    # ui 层（MainWindow + 8 个页面）
    src/ui/MainWindow.cpp
    src/ui/LoginPage.cpp
    src/ui/DashboardPage.cpp
    src/ui/ComponentPage.cpp
    src/ui/ProductVersionPage.cpp
    src/ui/ProductionBatchPage.cpp
    src/ui/RepairPage.cpp
    src/ui/TracePage.cpp
    src/ui/FeedbackPage.cpp
)

# 头文件（仅用于 IDE 导航，非必需但推荐）
set(HEADERS
    src/db/DatabaseManager.h

    src/db/repository/SupplierRepo.h
    src/db/repository/ComponentRepo.h
    src/db/repository/ComponentPriceRepo.h
    src/db/repository/EmployeeRepo.h
    src/db/repository/ProductRepo.h
    src/db/repository/ProductVersionRepo.h
    src/db/repository/VersionCostRepo.h
    src/db/repository/ProductBOMRepo.h
    src/db/repository/ProductionBatchRepo.h
    src/db/repository/BatchMaterialRepo.h
    src/db/repository/TestRecordRepo.h
    src/db/repository/CustomerRepo.h
    src/db/repository/RepairRecordRepo.h
    src/db/repository/RepairMaterialRepo.h
    src/db/repository/WarehouseRepo.h
    src/db/repository/InventoryRepo.h
    src/db/repository/InventoryTransactionRepo.h
    src/db/repository/ProductFeedbackRepo.h

    src/service/AuthService.h
    src/service/InventoryService.h
    src/service/ProductionService.h
    src/service/RepairService.h
    src/service/TraceService.h
    src/service/FeedbackService.h

    src/ui/MainWindow.h
    src/ui/LoginPage.h
    src/ui/DashboardPage.h
    src/ui/ComponentPage.h
    src/ui/ProductVersionPage.h
    src/ui/ProductionBatchPage.h
    src/ui/RepairPage.h
    src/ui/TracePage.h
    src/ui/FeedbackPage.h
)

# 可执行文件
qt_add_executable(HPLM ${SOURCES} ${HEADERS})

# 添加 src/ 为 include 目录，使 repo/service/ui 之间的 #include "db/..." 能正确解析
target_include_directories(HPLM PRIVATE ${CMAKE_SOURCE_DIR}/src)

# 链接 Qt6 库
target_link_libraries(HPLM PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Charts
    Qt6::Network
)

# 将 config 和 sql 目录复制到构建输出目录
add_custom_command(TARGET HPLM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/config" "$<TARGET_FILE_DIR:HPLM>/config"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/sql" "$<TARGET_FILE_DIR:HPLM>/sql"
    COMMENT "Copying config/ and sql/ to build output directory"
)

# 平台特定设置
if(WIN32)
    set_target_properties(HPLM PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
