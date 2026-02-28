#!/bin/bash

# 定义一个变量来存储提交信息
commit_message=${1:-"#module2-asm-s"}

# 执行命令
echo "Cleaning build files..."
make clean

echo "Adding all changes to git..."
git add .

echo "Committing changes with message: \"$commit_message\""
git commit -m "$commit_message"

echo "Pushing to the remote repository..."
git push

echo "Done!"