@echo off

node generateDocs.node.js

IF ERRORLEVEL 1 GOTO onError

goto onSuccess

:onError
pause

:onSuccess