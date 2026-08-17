// passman 核心逻辑单测：加密向量 / 载荷编解码 / 保存-打开往返 / 错误路径
#include <gtest/gtest.h>

#include <passman/crypto.h>
#include <passman/credfile.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace pm;

namespace {

Bytes fromHex(const std::string& hex) {
    Bytes out;
    out.reserve(hex.size() / 2);
    for(std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        out.push_back(static_cast<unsigned char>(
            std::stoul(hex.substr(i, 2), nullptr, 16)));
    }
    return out;
}

std::string toHex(const Bytes& b) {
    static const char* d = "0123456789abcdef";
    std::string s;
    s.reserve(b.size() * 2);
    for(unsigned char c : b) {
        s += d[c >> 4];
        s += d[c & 0xf];
    }
    return s;
}

fs::path tempFile(const char* name) {
    auto p = fs::temp_directory_path() / ("passman_test_" + std::string(name));
    std::error_code ec;
    fs::remove(p, ec);
    return p;
}

} // namespace

// —— 加密原语：公开测试向量 ——

TEST(Crypto, HmacSha256Rfc4231) {
    // Case 1: key = 0x0b×20, data = "Hi There"
    {
        Bytes key(20, 0x0b);
        Bytes data{'H', 'i', ' ', 'T', 'h', 'e', 'r', 'e'};
        EXPECT_EQ(toHex(hmacSha256(data, key)),
            "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7");
    }
    // Case 2: key = "Jefe", data = "what do ya want for nothing?"
    {
        Bytes key{'J', 'e', 'f', 'e'};
        const std::string msg = "what do ya want for nothing?";
        Bytes data(msg.begin(), msg.end());
        EXPECT_EQ(toHex(hmacSha256(data, key)),
            "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843");
    }
}

TEST(Crypto, Pbkdf2HmacSha256Vectors) {
    const Bytes salt{'s', 'a', 'l', 't'};
    EXPECT_EQ(toHex(deriveKey("password", salt, 1)),
        "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b");
    EXPECT_EQ(toHex(deriveKey("password", salt, 2)),
        "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43");
}

TEST(Crypto, Aes256CbcRoundtrip) {
    const Bytes key = randomBytes(32);
    const Bytes iv = randomBytes(16);
    const std::string msg = "hello world 中文 🎉 passman";
    Bytes plain(msg.begin(), msg.end());

    const Bytes cipher = encryptAes256Cbc(plain, key, iv);
    ASSERT_FALSE(cipher.empty());
    EXPECT_EQ(cipher.size() % 16, 0u);
    EXPECT_NE(cipher, plain);

    Bytes back;
    ASSERT_TRUE(decryptAes256Cbc(cipher, key, iv, back));
    EXPECT_EQ(back, plain);

    // 错误密钥 → 填充校验失败
    Bytes wrong = key;
    wrong[0] ^= 0xff;
    Bytes junk;
    EXPECT_FALSE(decryptAes256Cbc(cipher, wrong, iv, junk));

    // 非法输入：空密文 / 长度不对齐
    EXPECT_FALSE(decryptAes256Cbc({}, key, iv, junk));
    EXPECT_FALSE(decryptAes256Cbc(Bytes(17, 'x'), key, iv, junk));
}

TEST(Crypto, ConstantTimeEqual) {
    const Bytes a{1, 2, 3};
    EXPECT_TRUE(constantTimeEqual(a, Bytes{1, 2, 3}));
    EXPECT_FALSE(constantTimeEqual(a, Bytes{1, 2, 4}));
    EXPECT_FALSE(constantTimeEqual(a, Bytes{1, 2}));
}

TEST(Crypto, GeneratePassword) {
    GenOptions o;
    o.length = 20;
    o.upper = o.lower = o.digits = o.symbols = true;
    const std::string pw = generatePassword(o);
    ASSERT_EQ(pw.size(), 20u);
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSym = false;
    for(char c : pw) {
        if(c >= 'A' && c <= 'Z') hasUpper = true;
        else if(c >= 'a' && c <= 'z') hasLower = true;
        else if(c >= '0' && c <= '9') hasDigit = true;
        else hasSym = true;
    }
    // 20 字符全池采样，各类至少出现一个（概率上几乎必然）
    EXPECT_TRUE(hasUpper && hasLower && hasDigit && hasSym);

    // 空字符集 → 空串
    GenOptions none;
    none.upper = none.lower = none.digits = none.symbols = false;
    EXPECT_TRUE(generatePassword(none).empty());
    // 非法长度 → 空串
    GenOptions bad;
    bad.length = 0;
    EXPECT_TRUE(generatePassword(bad).empty());
    // 纯数字池：每位都是数字
    GenOptions digitsOnly;
    digitsOnly.upper = digitsOnly.lower = digitsOnly.symbols = false;
    for(char c : generatePassword(digitsOnly)) EXPECT_TRUE(c >= '0' && c <= '9');
}

TEST(Crypto, EstimateEntropyBits) {
    EXPECT_DOUBLE_EQ(estimateEntropyBits(""), 0.0);
    // 8 字符纯小写：8×log2(26)
    EXPECT_DOUBLE_EQ(estimateEntropyBits("aaaaaaaa"), 8.0 * std::log2(26.0));
    // 类别越多池越大
    const double lo = estimateEntropyBits("abcdefgh");
    const double mixed = estimateEntropyBits("Abcdefg1");
    EXPECT_GT(mixed, lo);
}

// —— 载荷编解码 ——

TEST(Payload, Roundtrip) {
    const std::vector<Entry> in{
        {"某公司", "user@example.com", "p@ss 中文密码"},
        {"GitHub", "alice", "S3cret!🎉"},
        {"", "bob", ""},
        {"只有公司", "", ""},
    };
    // 最后两条：账号密码全空 → 不应写入；第 3 条账号非空 → 保留
    const Bytes payload = encodePayload(in);
    auto out = decodePayload(payload);
    ASSERT_TRUE(out.has_value());
    ASSERT_EQ(out->size(), 3u);  // "只有公司" 被跳过
    EXPECT_EQ((*out)[0].company, "某公司");
    EXPECT_EQ((*out)[0].account, "user@example.com");
    EXPECT_EQ((*out)[0].password, "p@ss 中文密码");
    EXPECT_EQ((*out)[1].company, "GitHub");
    EXPECT_EQ((*out)[1].password, "S3cret!🎉");
    EXPECT_EQ((*out)[2].account, "bob");
}

TEST(Payload, EmptyAndTruncated) {
    // 空库：仅 count=0
    auto empty = decodePayload(encodePayload({}));
    ASSERT_TRUE(empty.has_value());
    EXPECT_TRUE(empty->empty());

    // 截断 → InvalidData
    const std::vector<Entry> one{{"c", "a", "p"}};
    auto payload = encodePayload(one);
    payload.pop_back();
    EXPECT_EQ(decodePayload(payload).error(), CredError::InvalidData);
    EXPECT_EQ(decodePayload({}).error(), CredError::InvalidData);
    // 尾部残留 → InvalidData
    auto withTail = encodePayload(one);
    withTail.push_back(0);
    EXPECT_EQ(decodePayload(withTail).error(), CredError::InvalidData);
}

// —— 文件容器 ——

TEST(Vault, SaveOpenRoundtrip) {
    const auto path = tempFile("roundtrip.cred");
    const std::vector<Entry> in{
        {"公司甲", "甲@example.com", "密码一"},
        {"Company B", "bob", "pw2"},
        {"", "carol", ""},
    };
    ASSERT_TRUE(saveVault(in, path, "主密码 test 主密码"));

    auto out = openVault(path, "主密码 test 主密码");
    ASSERT_TRUE(out.has_value()) << (int)out.error();
    // 第 3 条账号非空 → 保留；补一条全空条目验证跳过
    ASSERT_EQ(out->entries.size(), 3u);
    EXPECT_EQ(out->entries[0].company, "公司甲");
    EXPECT_EQ(out->entries[0].password, "密码一");
    EXPECT_EQ(out->entries[1].account, "bob");
    EXPECT_EQ(out->entries[2].account, "carol");
    EXPECT_TRUE(saveVault({{"将被跳过", "", ""}}, path, "主密码 test 主密码"));
    auto skip = openVault(path, "主密码 test 主密码");
    ASSERT_TRUE(skip.has_value());
    EXPECT_TRUE(skip->entries.empty());  // 账号与密码全空的条目不写入
    EXPECT_EQ(out->iterations, kDefaultIterations);

    fs::remove(path);
}

TEST(Vault, WrongPassword) {
    const auto path = tempFile("wrongpw.cred");
    ASSERT_TRUE(saveVault({{"c", "a", "p"}}, path, "correct horse"));
    auto out = openVault(path, "wrong battery");
    ASSERT_FALSE(out.has_value());
    EXPECT_EQ(out.error(), CredError::BadPassword);
    fs::remove(path);
}

TEST(Vault, TamperedCiphertext) {
    const auto path = tempFile("tamper.cred");
    ASSERT_TRUE(saveVault({{"c", "a", "p"}}, path, "master"));
    // 篡改密文中部一个字节 → HMAC 失败（错密码路径）
    std::ifstream f(path, std::ios::binary);
    Bytes data{std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
    f.close();
    data[data.size() / 2] ^= 0x5a;
    std::ofstream g(path, std::ios::binary | std::ios::trunc);
    g.write(reinterpret_cast<const char*>(data.data()), (std::streamsize)data.size());
    g.close();
    EXPECT_EQ(openVault(path, "master").error(), CredError::BadPassword);
    fs::remove(path);
}

TEST(Vault, FormatErrors) {
    // 随机垃圾 → Format
    const auto junk = tempFile("junk.cred");
    {
        std::ofstream f(junk, std::ios::binary);
        f << "not a credential file at all, just some text........";
    }
    EXPECT_EQ(openVault(junk, "x").error(), CredError::Format);
    fs::remove(junk);

    // 截断的合法文件 → Format
    const auto trunc = tempFile("trunc.cred");
    ASSERT_TRUE(saveVault({{"c", "a", "p"}}, trunc, "m"));
    std::ifstream f2(trunc, std::ios::binary);
    Bytes data{std::istreambuf_iterator<char>(f2), std::istreambuf_iterator<char>()};
    f2.close();
    data.resize(data.size() - 10);
    std::ofstream g2(trunc, std::ios::binary | std::ios::trunc);
    g2.write(reinterpret_cast<const char*>(data.data()), (std::streamsize)data.size());
    g2.close();
    EXPECT_EQ(openVault(trunc, "m").error(), CredError::Format);
    fs::remove(trunc);

    // 不存在的文件 → Io
    EXPECT_EQ(openVault(fs::temp_directory_path() / "passman_test_no_such.cred", "x").error(),
        CredError::Io);
}

TEST(Vault, InvalidArgs) {
    const auto path = tempFile("empty.cred");
    // 空主密码拒绝
    EXPECT_EQ(saveVault({{"c", "a", "p"}}, path, "").error(), CredError::InvalidArg);
    // 迭代数过低拒绝
    EXPECT_EQ(saveVault({{"c", "a", "p"}}, path, "m", 10).error(), CredError::InvalidArg);
    fs::remove(path);
}

TEST(Vault, EmptyVaultFile) {
    const auto path = tempFile("emptyvault.cred");
    ASSERT_TRUE(saveVault({}, path, "m"));
    auto out = openVault(path, "m");
    ASSERT_TRUE(out.has_value());
    EXPECT_TRUE(out->entries.empty());
    fs::remove(path);
}

// —— 保存随机性：同输入两次保存，salt/iv 不同 → 文件不同 ——

TEST(Vault, UniqueSaltIvPerSave) {
    const auto a = tempFile("rand1.cred");
    const auto b = tempFile("rand2.cred");
    ASSERT_TRUE(saveVault({{"c", "a", "p"}}, a, "m"));
    ASSERT_TRUE(saveVault({{"c", "a", "p"}}, b, "m"));
    EXPECT_NE(fs::file_size(a), 0u);
    std::ifstream fa(a, std::ios::binary), fb(b, std::ios::binary);
    std::string sa((std::istreambuf_iterator<char>(fa)), {});
    std::string sb((std::istreambuf_iterator<char>(fb)), {});
    fa.close();
    fb.close();
    EXPECT_NE(sa, sb);
    fs::remove(a);
    fs::remove(b);
}
