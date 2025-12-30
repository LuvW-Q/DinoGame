# 分数加密存储实现说明

## 概述
本项目实现了对游戏最高分数的加密存储功能，使用对称加密算法保护用户数据安全。

## 加密配置

### 密钥配置
密钥定义在 `gameconfig.h` 中：
```cpp
const QString ENCRYPTION_KEY = "ee7d5971-c06e-485d-8b09-abae73aef66d";
const QString HIGHSCORE_FILE = "highscore.dat";
```

### 存储位置
- **文件位置**: `{AppDataLocation}/highscore.dat`
- **AppDataLocation** 根据操作系统自动确定：
  - Windows: `C:\Users\{UserName}\AppData\Roaming\{AppName}`
  - macOS: `/Users/{UserName}/Library/Application Support/{AppName}`
  - Linux: `~/.local/share/{AppName}`

## 加密算法

### 加密流程
1. 将分数转换为字符串
2. 使用提供的密钥生成 SHA-256 哈希值
3. 使用 XOR 操作对分数数据进行加密（使用 SHA-256 哈希作为密钥流）
4. 将加密后的字节转换为十六进制字符串存储

```cpp
QString encryptScore(int score) {
    QString scoreStr = QString::number(score);
    QByteArray key = GameConfig::ENCRYPTION_KEY.toUtf8();
    QByteArray keyHash = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
    
    QByteArray scoreData = scoreStr.toUtf8();
    QByteArray encrypted;
    
    for (int i = 0; i < scoreData.length(); ++i) {
        encrypted.append(scoreData[i] ^ keyHash[i % keyHash.length()]);
    }
    
    return QString::fromLatin1(encrypted.toHex());
}
```

### 解密流程
1. 将十六进制字符串转换回字节
2. 生成相同的 SHA-256 哈希值
3. 使用 XOR 操作解密（XOR 的逆操作仍是 XOR）
4. 将解密后的字节转换为字符串并解析为整数

```cpp
int decryptScore(const QString &encrypted) {
    QByteArray key = GameConfig::ENCRYPTION_KEY.toUtf8();
    QByteArray keyHash = QCryptographicHash::hash(key, QCryptographicHash::Sha256);
    
    QByteArray encryptedData = QByteArray::fromHex(encrypted.toLatin1());
    QByteArray decrypted;
    
    for (int i = 0; i < encryptedData.length(); ++i) {
        decrypted.append(encryptedData[i] ^ keyHash[i % keyHash.length()]);
    }
    
    QString decryptedStr = QString::fromUtf8(decrypted);
    bool ok;
    int score = decryptedStr.toInt(&ok);
    
    return ok ? score : -1;
}
```

## 使用流程

### 加载最高分
在游戏窗口初始化时调用 `loadHighScore()`：
```cpp
GameWindow::GameWindow(QWidget *parent) : QWidget(parent) {
    // ...其他初始化代码...
    highScore = 0;
    loadHighScore(); // 加载加密的高分文件
}
```

### 保存最高分
当游戏结束且当前分数超过历史最高分时调用 `saveHighScore()`：
```cpp
if (checkCollision()) {
    isGameOver = true;
    if (score > highScore) {
        highScore = score;
        saveHighScore(); // 保存加密的高分数据
    }
}
```

## 文件结构

修改的文件：
- `src/gameconfig.h` - 添加加密密钥和文件名配置
- `src/gamewindow.h` - 添加加密/解密方法声明
- `src/gamewindow.cpp` - 实现加密/解密方法和修改读写高分逻辑

新增文件：
- `test_encryption.cpp` - 加密功能测试程序

## 安全性说明

### 优点
- ✓ 防止用户通过直接编辑文件修改高分
- ✓ 使用 SHA-256 哈希生成密钥，增强安全性
- ✓ 存储为十六进制编码，人类不可读

### 局限性
- ⚠ XOR 加密相比现代的对称加密算法（如 AES）强度较低
- ⚠ 密钥硬编码在代码中，反编译可获取
- ⚠ 不适合保护高度敏感的数据

### 改进建议
如需更高安全性，可以：
1. 使用 OpenSSL 库实现真正的 AES-256 加密
2. 从外部配置文件读取密钥
3. 使用更复杂的密钥派生函数（KDF）

## 测试

运行测试程序验证加密功能：
```bash
# 使用 Qt 编译器编译测试程序
# 需要链接 Qt Core 库
```

示例输出：
```
=== 分数加密解密测试 ===
使用密钥: ee7d5971-c06e-485d-8b09-abae73aef66d

原始分数: 0 | 加密: ... | 解密: 0 | 结果: ✓ 成功
原始分数: 100 | 加密: ... | 解密: 100 | 结果: ✓ 成功
原始分数: 999 | 加密: ... | 解密: 999 | 结果: ✓ 成功
```

## 依赖

- Qt 6.0+
- C++ 17+
- QCryptographicHash（Qt Core 库内置）

## 注意事项

1. **初始化**: 首次运行游戏时，如果高分文件不存在，会自动创建
2. **错误处理**: 如果加密/解密失败，分数会被设为 -1（无效）或 0（默认值）
3. **跨平台**: 加密算法和文件格式在所有平台上一致，可安全迁移


