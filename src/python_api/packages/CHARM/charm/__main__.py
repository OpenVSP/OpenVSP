# Copyright (c) 2023 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.  All Other
# Rights Reserved.

# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

from charm import copyright


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description='OpenVSP-to-CHARM Automation', prog='charm'
    )

    parser.add_argument(
        '--copyright', help='output full copyright statement', action='store_true', default=False
    )

    options = parser.parse_args()
    verbose = getattr(options, 'copyright', False)

    if verbose:
        copyright(long=True)
    else:
        print("OpenVSP-to-CHARM is not independent executable package.  See documentation for correct usage.")
        parser.print_help()
