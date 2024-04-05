# 存档管理器64位（开发中）

## 已测试兼容的游戏
- **Noita**：几乎可以随时存档，必须在关闭游戏后读档。
- **暖雪 Warm Snow**：可在游戏非加载时段存/读档。
- **我的世界 Minecraft**：备份时不能游玩对应存档（否则会因不可抗力崩溃，并留下一个无效存档）。
- **Balatro**: 完全兼容，但必须退回主菜单后再回档。

## 使用方法
1. **创建新存档项目**：`File` -> `New Backup`添加新存档项目，请正确设置文件路径。路径可以包含UTF-8字符。
2. **快速存/读档**：
   - 点击`QS`进行快速存档，会自动将当前时间作为存档名称。
   - 点击`QL`进行快速读档，会选择最近一次存档来读档。
   - 点击`S`，`L`进行普通存/读档。
   
> **警告**：由于目前仍处于测试版本，请额外备份一份源文件以免误删。

## TODO
- [x] 加入压缩功能。
- [ ] 加入中/英语言切换支持。

## 资源
<a target="_blank" href="https://icons8.com/icon/774/list">List</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a><br>
<a target="_blank" href="https://icons8.com/icon/33277/merge-git">Branch</a> icon by <a target="_blank" href="https://icons8.com">Icons8</a>

## 其它
- 使用了7z.exe，来自https://7-zip.org/
- 使用Qt5框架
