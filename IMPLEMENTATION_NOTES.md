# 离散数学命题系统 - 实现说明

## 已完成的功能模块

### 1. 核心架构

#### 数据库层 (Database)
- `database.h/cpp` - SQLite数据库管理单例类
- 自动创建表结构
- 事务管理支持
- 连接池管理

#### 数据模型 (Models)
- `models.h` - 所有数据结构定义
  - User - 用户模型
  - Question - 题目模型
  - Paper - 试卷模型
  - PaperConfig - 试卷配置模型
  - QuestionType枚举 - 题型枚举

#### 数据访问层 (DAO)
- `userdao.h/cpp` - 用户数据访问
- `questiondao.h/cpp` - 题目数据访问
- `paperdao.h/cpp` - 试卷数据访问

#### 业务逻辑层 (Services)
- `authservice.h/cpp` - 认证服务（注册、登录）
- `questionservice.h/cpp` - 题目管理服务
- `paperservice.h/cpp` - 试卷管理服务

#### AI模块
- `aiclient.h/cpp` - DeepSeek API客户端
- `ai_config.h` - AI配置文件
- 支持题目生成、质量分析、智能推荐

#### 工具类
- `jsonutils.h/cpp` - JSON序列化工具
- `passwordutils.h/cpp` - 密码加密（PBKDF2）

### 2. UI组件

#### 主窗口 (MainWindow)
- `mainwindow.h/cpp/ui` - 主应用程序窗口
- 菜单栏系统
- 多页面切换（登录、题库、试卷、个人信息）
- 状态栏

#### 登录窗口 (LoginWidget)
- `loginwidget.h/cpp/ui` - 用户登录界面
- 集成AuthService

#### 题库管理 (Widget)
- `widget.h/cpp/ui` - 题目浏览和管理
- 题目筛选（按类型、按难度）
- 题目详情显示
- 统计信息

### 3. 配置文件

#### 应用配置
- `config/config.h` - 应用级配置
- 数据库路径
- API配置
- 安全参数

#### AI配置
- `config/ai_config.h` - AI服务配置
- API密钥
- 提示词模板

## 技术特点

### 数据库设计
- SQLite3轻量级数据库
- 外键约束
- 索引优化
- 软删除（status字段）

### 安全性
- PBKDF2密码哈希
- 密码强度验证
- 会话管理
- SQL参数化查询

### AI集成
- DeepSeek API集成
- 异步网络请求
- JSON响应解析
- 错误处理

### 代码架构
- MVC架构模式
- 单例模式（Database）
- 信号槽通信
- 清晰的分层设计

## 待完成功能

### 短期目标
1. **注册界面** - 完整的用户注册UI
2. **题目编辑** - 题目创建和编辑界面
3. **试卷创建** - 试卷配置和创建界面
4. **PDF导出** - 完善的试卷导出功能

### 中期目标
1. **题目导入/导出** - JSON格式的批量操作
2. **试卷预览** - Web富文本预览
3. **用户信息管理** - 个人资料编辑
4. **头像上传** - 文件处理功能

### 长期目标
1. **多科目支持** - 扩展到其他学科
2. **协作功能** - 多用户协同编辑
3. **数据分析** - 学习效果统计
4. **移动端** - Qt Quick跨平台应用

## 使用说明

### 首次运行

1. 打开Qt Creator
2. 加载 `propositional_system.pro`
3. 点击"运行"按钮

程序将自动：
- 创建 `data/` 目录
- 创建 `database.db` 文件
- 初始化所有数据表

### 登录系统

由于尚未实现注册界面，需要手动插入测试用户：

```sql
-- 在数据库中手动插入测试用户
INSERT INTO users (username, email, password_hash, role)
VALUES ('admin', 'admin@example.com', '生成的哈希', 2);
```

或使用程序内置的简单生成逻辑（当前未实现）

### AI功能

要使用AI功能，需要：
1. 在 `config/ai_config.h` 中配置API密钥
2. 确保网络连接正常
3. API配额充足

## 编译注意事项

### Qt版本要求
- Qt 6.5 或更高版本
- 支持 MinGW 或 MSVC 编译器

### 依赖模块
- `sql` - SQLite支持
- `network` - 网络请求
- `printsupport` - PDF导出

### 可能的问题

1. **数据库驱动**
   - 确保安装了SQLite驱动：`Qt6Sql/plugins/sqldrivers/qsqlite.dll`

2. **网络请求**
   - Windows可能需要手动部署OpenSSL库

3. **中文编码**
   - 确保源文件使用UTF-8编码

## 调试建议

### 日志输出
```cpp
qDebug() << "Debug message";
qWarning() << "Warning message";
qCritical() << "Critical message";
```

### 数据库检查
```sql
-- 查看所有表
.tables

-- 查看表结构
.schema questions

-- 查询用户
SELECT * FROM users;
```

### 常见错误
1. "Driver not loaded" - 检查SQLite插件
2. "Database locked" - 关闭其他数据库连接
3. "Network error" - 检查网络和API密钥

## 性能优化建议

1. **数据库**
   - 为常用查询添加索引
   - 使用事务批量操作
   - 考虑数据库连接池

2. **网络请求**
   - 实现请求缓存
   - 限制并发请求数
   - 添加超时控制

3. **UI渲染**
   - 使用分页加载大量数据
   - 异步加载图片
   - 优化列表滚动性能

## 扩展开发

### 添加新题型
1. 在 `models.h` 的 `QuestionType` 枚举中添加
2. 更新 `questionTypeToString()` 函数
3. 在题目生成函数中添加逻辑

### 添加新数据表
1. 修改 `database.cpp` 的 `createTables()`
2. 创建对应的DAO类
3. 在Service中集成

### 添加新AI功能
1. 在 `aiclient.h` 中添加方法
2. 在 `ai_config.h` 中添加提示词模板
3. 在Service中调用

## 许可证

MIT License

## 贡献者

项目维护团队
