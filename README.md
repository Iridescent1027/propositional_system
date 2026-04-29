# 离散数学命题系统

基于Qt 6 + qmake + SQLite的智能命题系统。

## 功能特性

- **用户管理**: 注册、登录、个人信息管理
- **题目管理**: 题目CRUD、题库管理、题型分类
- **试卷管理**: 试卷生成、配置管理、PDF导出
- **AI智能**: AI命题生成、题目质量分析、智能推荐

## 技术栈

- Qt 6.5+
- SQLite3
- C++17
- DeepSeek API (AI功能)

## 项目结构

```
propositional_system/
├── src/
│   ├── config/           # 配置文件
│   ├── database.h/cpp    # 数据库管理
│   ├── models.h          # 数据模型
│   ├── jsonutils.h/cpp   # JSON工具
│   ├── passwordutils.h/cpp # 密码工具
│   ├── dao/              # 数据访问对象
│   ├── services/         # 业务逻辑服务
│   ├── ai/               # AI客户端
│   ├── widgets/          # UI组件
│   └── main.cpp          # 程序入口
├── data/                 # 数据目录
│   └── database.db       # SQLite数据库
└── propositional_system.pro # qmake项目文件
```

## 编译运行

### 环境要求

- Qt 6.5 或更高版本
- MinGW 或 MSVC 编译器
- CMake 或 qmake

### 编译步骤

1. 使用Qt Creator打开项目
   ```bash
   qtcreator propositional_system.pro
   ```

2. 或使用命令行编译
   ```bash
   qmake propositional_system.pro
   mingw32-make  # Windows
   # 或
   make          # Linux/Mac
   ```

3. 运行程序
   ```bash
   ./propositional_system
   ```

## 使用说明

### 首次使用

1. 启动程序后，点击"注册"按钮创建用户账号
2. 用户名至少3个字符
3. 密码必须包含大小写字母和数字，至少6个字符

### 题目管理

- 查看题库中的所有题目
- 按类型筛选（命题逻辑、集合论、图论、关系）
- 按难度筛选（1-5级）
- 点击题目列表查看详情

### AI功能

AI功能需要配置API密钥，配置文件位于：
- `src/config/ai_config.h`

修改 `DEEPSEEK_API_KEY` 为你的API密钥。

## 数据库

数据库文件位于 `data/database.db`，首次运行会自动创建并初始化表结构。

包含的表：
- `users` - 用户表
- `questions` - 题目表
- `papers` - 试卷表
- `paper_questions` - 试卷题目关联表

## 开发说明

### 添加新功能

1. 在 `src/models.h` 中定义数据模型
2. 在 `src/dao/` 中创建数据访问对象
3. 在 `src/services/` 中创建业务逻辑服务
4. 在 `src/widgets/` 中创建UI组件
5. 更新 `propositional_system.pro` 添加新文件

### 代码风格

- 使用驼峰命名法 (camelCase)
- 类名首字母大写
- 私有成员变量使用 `m_` 前缀
- 遵循Qt编码规范

## 待完成功能

- [ ] 用户注册界面
- [ ] 题目编辑界面
- [ ] 试卷创建界面
- [ ] PDF导出功能优化
- [ ] 题目导入/导出
- [ ] 更多UI组件完善

## 许可证

MIT License

## 联系方式

如有问题或建议，请联系项目维护者。
