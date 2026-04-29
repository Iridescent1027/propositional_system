#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

namespace Config {

// 数据库配置
inline const QString DB_PATH = "data/database.db";
inline const QString DB_INIT_SCRIPT = "data/init_database.sql";

// AI配置 - DeepSeek API
inline const QString DEEPSEEK_API_KEY = "sk-79498c1b6d6049388871c93909beb826";
inline const QString DEEPSEEK_API_URL = "https://api.deepseek.com/v1/chat/completions";
inline const QString DEEPSEEK_MODEL = "deepseek-chat";

// 应用配置
inline const QString APP_NAME = "离散数学命题系统";
inline const QString APP_VERSION = "1.0.0";
inline const QString ORGANIZATION_NAME = "PropositionalSystem";

// 安全配置
inline const int MAX_LOGIN_ATTEMPTS = 5;
inline const int SESSION_TIMEOUT_MINUTES = 30;
inline const int PASSWORD_MIN_LENGTH = 6;

// 分页配置
inline const int DEFAULT_PAGE_SIZE = 20;

// 文件配置
inline const QString DATA_DIR = "data";
inline const QString AVATAR_DIR = "data/avatars";
inline const QString EXPORT_DIR = "data/exports";

} // namespace Config

#endif // CONFIG_H
