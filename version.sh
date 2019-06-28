#!/bin/bash

RETVAL=0

. check_lock.sh

check_lock
RETVAL=$?

if [[ ${RETVAL} -ne 0 ]]; then
    exit 0;
fi

PROTO_FILE="$(dirname $0)/libs/libupmqprotocol/protocol.proto"

SERVER_NAME=""
SERVER_MAJOR=""
SERVER_MINOR=""
CLIENT_MAJOR=""
CLIENT_MINOR=""

get_versions()
{
    OLD_IFS=$IFS      # save the field separator
    IFS=$'\n'     # new field separator, the end of line

    echo "protofile path ${1}"

    for line in $(cat ${1})
    do
        case ${line} in
            *"server_vendor_id"*)
                SERVER_NAME=$(sed -e 's#.*=\(\)#\1#' <<< "${line}")
                SERVER_NAME=$(sed -e 's/^[^"]*"//; s/".*//' <<< "${SERVER_NAME}")
            ;;
            *"server_major_version"*)
                SERVER_MAJOR=$(sed -e 's#.*=\(\)#\1#' <<< "${line}")
                SERVER_MAJOR=$(sed -e 's/[^0-9]*//g' <<< "${SERVER_MAJOR}")
            ;;
            *"server_minor_version"*)
                SERVER_MINOR=$(sed -e 's#.*=\(\)#\1#' <<< "${line}")
                SERVER_MINOR=$(sed -e 's/[^0-9]*//g' <<< "${SERVER_MINOR}")
            ;;
            *"client_major_version"*)
                CLIENT_MAJOR=$(sed -e 's#.*=\(\)#\1#' <<< "${line}")
                CLIENT_MAJOR=$(sed -e 's/[^0-9]*//g' <<< "${CLIENT_MAJOR}")
            ;;
            *"client_minor_version"*)
                CLIENT_MINOR=$(sed -e 's#.*=\(\)#\1#' <<< "${line}")
                CLIENT_MINOR=$(sed -e 's/[^0-9]*//g' <<< "${CLIENT_MINOR}")
            ;;
        esac
    done

    if [[ -z ${SERVER_NAME} ]]; then
        SERVER_NAME=upmq;
    fi
    if [[ -z ${SERVER_MAJOR} ]]; then
        SERVER_MAJOR=0;
    fi
    if [[ -z ${SERVER_MINOR} ]]; then
        SERVER_MINOR=0;
    fi
    if [[ -z ${CLIENT_MAJOR} ]]; then
        CLIENT_MAJOR=0;
    fi
    if [[ -z ${CLIENT_MINOR} ]]; then
        CLIENT_MINOR=0;
    fi

    IFS=${OLD_IFS}     # restore default field separator
}

get_versions ${PROTO_FILE}

if which git >/dev/null; then
    FILE_NAME_VER_PROP="$(dirname $0)/VERSION.txt"
    FILE_NAME_COMMIT_PROP="$(dirname $0)/COMMIT.txt"
    FILE_NAME="$(dirname $0)/share/Version.hpp"
    rm -f ${FILE_NAME}
    BUILD_VER=$(git log --oneline | wc -l)
    BUILD_VER=$(echo -e ${BUILD_VER} | tr -d '[[:space:]]')
    COMMITTER_FULLSHA=$(git log -n 1 "--pretty=format:%H")
    COMMITTER_SHORTSHA=$(git log -n 1 "--pretty=format:%h")
    COMMITTER_NAME=$(git log -n 1 "--pretty=format:%cn")
    COMMITTER_EMAIL=$(git log -n 1 "--pretty=format:%ce")
    COMMITTER_DATE=$(git log -n 1 "--pretty=format:%ci")
    COMMITTER_NOTE=$(git log -n 1 "--pretty=format:%s")

    echo "SERVER NAME    : ${SERVER_NAME}"
    echo "SERVER VERSION : ${SERVER_MAJOR}.${SERVER_MINOR}.${BUILD_VER}"

    echo "CLIENT VERSION : ${CLIENT_MAJOR}.${CLIENT_MINOR}.${BUILD_VER}"

    echo "#ifndef VERSION_H" >> ${FILE_NAME}
    echo "#define VERSION_H" >> ${FILE_NAME}
    echo "" >> ${FILE_NAME}
    echo "#include <string>" >> ${FILE_NAME}
    echo "#include <sstream>" >> ${FILE_NAME}
    echo "" >> ${FILE_NAME}
    echo "" >> ${FILE_NAME}
    echo "#define MQ_VERSION_MAJOR      ${SERVER_MAJOR}" >> ${FILE_NAME}
    echo "#define MQ_VERSION_MINOR      ${SERVER_MINOR}" >> ${FILE_NAME}
    echo "#define MQ_VERSION_REVISION   ${BUILD_VER}" >> ${FILE_NAME}
    echo "" >> ${FILE_NAME}
    echo "#define MQ_COMMITTER_NAME     \"$COMMITTER_NAME\"" >> ${FILE_NAME}
    echo "#define MQ_COMMITTER_EMAIL    \"$COMMITTER_EMAIL\"" >> ${FILE_NAME}
    echo "" >> ${FILE_NAME}
    echo "#define MQ_COMMITTER_FULLSHA	\"$COMMITTER_FULLSHA\"" >> ${FILE_NAME}
    echo "#define MQ_COMMITTER_SHORTSHA	\"$COMMITTER_SHORTSHA\"" >> ${FILE_NAME}
    echo "#define MQ_COMMITTER_DATE		\"$COMMITTER_DATE\"" >> ${FILE_NAME}
    echo "#define MQ_COMMITTER_NOTE		\"$COMMITTER_NOTE\"" >> ${FILE_NAME}
    echo "" >> ${FILE_NAME}
    echo "#endif // VERSION_H" >> ${FILE_NAME}
    echo "Version file  : ${FILE_NAME}"
    echo "Version number: ${BUILD_VER}"
    echo "COMMITTER SHA  : $COMMITTER_FULLSHA"
    echo "COMMITTER SHA  : $COMMITTER_SHORTSHA"
    echo "COMMITTER name : $COMMITTER_NAME"
    echo "COMMITTER email: $COMMITTER_EMAIL"
    echo "COMMITTER date : $COMMITTER_DATE"

    rm -f ${FILE_NAME_VER_PROP}
    echo "${SERVER_MAJOR}.${SERVER_MINOR}.${BUILD_VER}" >> ${FILE_NAME_VER_PROP}

    rm -f ${FILE_NAME_COMMIT_PROP}
    echo "${COMMITTER_SHORTSHA}" >> ${FILE_NAME_COMMIT_PROP}

else
	echo "{--------------------------------------------------}"
	echo "{                                                  }"
	echo "{                                                  }"
	echo "{ Please set the Path variable in your environment }"
	echo "{           to match the location of git           }"
	echo "{                                                  }"
	echo "{--------------------------------------------------}"
fi