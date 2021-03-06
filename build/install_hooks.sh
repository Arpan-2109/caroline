#!/usr/bin/env bash
# Copyright (c) 2014 The Caroline authors. All rights reserved.
# Use of this source file is governed by a MIT license that can be found in the
# LICENSE file.
# Author: Aleksandr Derbenev <alex@technoworks.ru>

# Install script for cpplint run into the pre-commit git hook.
cat > .git/hooks/pre-commit << EOF
#!/usr/bin/env bash
# Copyright (c) 2014 The Caroline authors. All rights reserved.
# Use of this source file is governed by a MIT license that can be found in the
# LICENSE file.
# Author: Aleksandr Derbenev <alex@technoworks.ru>

exec python ./build/lint_all.py .

EOF

chmod +x .git/hooks/pre-commit

