### 第一部分：最终合并指南（把成品放入 main）

**请在项目结束（1月16日前）按以下步骤操作：**

1.  **切换到 main 分支：**
    ```bash
    git checkout main
    ```

2.  **以 Source 分支为主体进行合并（保留代码）：**
    这步操作会将 main 变成和 source 一样（只有代码，没有文档）。
    ```bash
    git merge source
    # 如果提示冲突，选择接受 source 的变动
    ```

3.  **从 Doc 分支“捡回”文档：**
    不进行合并，而是直接从 `doc` 分支把文档文件“抓”过来。
    ```bash
    # 语法：git checkout [分支名] -- [文件或文件夹路径]
    ```

4.  **提交最终版本：**
    现在工作区里既有代码（来自 merge），又有文档（刚刚抓过来的）。
    ```bash
    git add .
    git commit -m "Release: 合并最终代码与文档"
    git push origin main
    ```

---

### 第二部分：团队 Git协作与开发指南

---

# 项目 Git 协作指南

## 1. 项目分支结构
仓库分为三个主要分支：

*   **`main` (主分支)**：仅用于最终发布。日常不要直接在 main 上修改。
*   **`source` (代码分支)**：**程序开发人员**的主战场。这里只有代码，没有大体积文档。
*   **`doc` (文档分支)**：**文档编写人员**的主战场。这里只有文档和模型，没有代码。

---

## 2. 如何开始工作

```bash
# 1. 克隆仓库
git clone https://github.com/lanse69/ShortVideo-DevLifecycle.git

# 2. 进入目录
cd ShortVideo-DevLifecycle

# 3. 切换分支
#写代码的：
git checkout source

#写文档的：
git checkout doc
```

---

## 3. 日常流程

请严格遵守 **"先拉后推"** 的原则。

### 场景 A：写代码的 (在 source 分支)

1.  **开始工作前，更新代码：**
    ```bash
    git checkout source
    git pull origin source
    ```
2.  **提交更改：**
    ```bash
    git add .
    git commit -m "具体的提交信息"
    git pull origin source  # 再次防止冲突
    ```
3.  **推送到仓库：**
    ```bash
    git push origin source
    ```

### 场景 B：写文档的 (在 doc 分支)

1.  **开始工作前：**
    ```bash
    git checkout doc
    git pull origin doc
    ```
3.  **提交与推送：**
    ```bash
    git add .
    git commit -m "Docs: 更新需求分析规格说明书 v1.2"
    git push origin doc
    ```

---

## 4. 遇到冲突怎么办？(Conflict)

在 `git pull` 或 `git merge` 时看到 `CONFLICT` 字样：

1.  打开提示冲突的文件（代码文件）。
2.  你会看到 `<<<<<<< HEAD` 和 `>>>>>>>` 的标记。
3.  **手动修改**：保留需要的代码，删除 Git 自动生成的标记符号。
4.  **重新提交**：
    ```bash
    git add .
    git commit -m "Fix: 解决代码合并冲突"
    git push
    ```

---

## 5. 提交信息规范 (Commit Message)

请保持提交记录清晰，建议格式：`类型: 描述`。

*   `Feat: ...` (新功能) -> 例：`Feat: 增加用户点赞接口`
*   `Fix: ...` (修Bug) -> 例：`Fix: 修复登录失败崩溃的问题`
*   `Docs: ...` (文档) -> 例：`Docs: 完成详细设计第四章`
*   `Style: ...` (格式) -> 例：`Style: 调整代码缩进`
*   `Refactor: ...` (重构) -> 例：`Refactor: 优化数据库连接池`
