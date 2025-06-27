#!/bin/bash

#
# cmsfinder.sh - finds popular software installed in a cPanel account.
#
# Searches directories for index.php and .htaccess files and checks if
# they contain strings from various software, then checks for files that
# contain that software's version information.
#
# The script doesn't work correctly on directories with spaces in them!
#

_DEBUG="off"
function DEBUG() {
  [ "$_DEBUG" == "on" ] && echo "debug: $@"
}

function usage() {
    cat << EOF
Usage: $0 [options] username
Finds common software applications installed for the user <username>. 

Currently supports: WordPress, Drupal, Magento, PrestaShop, Joomla,
    Concrete, phpBB, MediaWiki, DocuWiki, WHMCS, and Laravel, as well as
    Node.js and Python apps.

Options:
-d | --depth   depth to search for apps. Defaults to 1,
               meaning it will search document roots and 
               the directories immediately under it, like
               /home/user/domain.com/directory
               
EOF
}

# Set colors
red="\\033[1;31m"
cyan="\\033[1;36m"
white="\\033[1;37m"
yellow="\\033[1;33m"
magenta="\\033[1;35m"
green="\\033[1;32m"
normal="\\033[0m"

# apps to search for (used to grep index.php and .htaccess files)
apps=("WordPress" "Drupal" "Magento" "PrestaShop" "Joomla" "concrete" "phpBB" 
"MediaWiki" "doku.php" "LARAVEL" "PassengerNodejs" "PassengerPython" "WHMCS")

# array to store directories to check for apps in
docroots=()

# default depth to search directories under document roots
searchdepth=1

user=""

function findapps() {
    mainroot=($(uapi --output=jsonpretty --user="${user}" DomainInfo domains_data 2>/dev/null | jq -r '.result.data.main_domain.documentroot'))
    if [ -z "${mainroot}" ]; then
        # we didnt' get a main domain docroot, so this isn't a valid cPanel user
        echo -ne "${magenta}No primary document root found!\n"
        echo -ne "This isn't a valid cPanel user!${normal}\n"
        exit;
    fi

    # primary domain docroot
    for directory in $(find "${mainroot}" -maxdepth "${searchdepth}" -type d)
    do
        docroots+=("${directory}")
    done

    # addon domain docroots
    for docroot in $(uapi --output=jsonpretty --user="${user}" DomainInfo domains_data | jq -r '.result.data.addon_domains[].documentroot');
    do
        for directory in $(find "${docroot}" -maxdepth "${searchdepth}" -type d)
        do
            docroots+=("${directory}")
        done
    done

    # subdomain docroots
    for docroot in $(uapi --output=jsonpretty --user="${user}" DomainInfo domains_data | jq -r '.result.data.sub_domains[].documentroot');
    do
        for directory in $(find "${docroot}" -maxdepth "${searchdepth}" -type d)
        do
            docroots+=("${directory}")
        done
    done

    DEBUG "Directories  :"
    for docroot in "${docroots[@]}"
    do
        DEBUG "${docroot}"
    done
    DEBUG ""

    for docroot in "${docroots[@]}"
    do
        DEBUG "Checking ${docroot}..."
        index_content=""
        htaccess_content=""

        if [ -f "${docroot}/index.php" ]; then
            index_content=$(<${docroot}/index.php)
            DEBUG "index.php found"
        fi

        if [ -f "${docroot}/pub/index.php" ]; then
            index2_content=$(<${docroot}/pub/index.php)
            DEBUG "pub/index.php found"
            index_content=${index_content}+${index2_content}
        fi

        if [ -f "${docroot}/public/index.php" ]; then
            index2_content=$(<${docroot}/public/index.php)
            DEBUG "public/index.php found"
            index_content=${index_content}+${index2_content}
        fi

        if [ -f "${docroot}/.htaccess" ]; then
            DEBUG ".htaccess found"
            htaccess_content=$(<${docroot}/.htaccess)
        fi

        if [ -z "${index_content}" ] && [ -z "${htaccess_content}" ]; then
            # no index.php or .htaccess files found, skip to the next directory
            DEBUG "no index.php or .htaccess files found"
            continue
        fi
        
        for app in ${apps[@]}
        do
            DEBUG "Checking for app ${app}"
            if [[ "${index_content}" == *$app* ]] || [[ "${htaccess_content}" == *$app* ]]; then
                DEBUG "Detected $app"

                case "$app" in
                    WordPress)
                        # retrieve the version string from wp-includes/version.php
                        if [ -f "${docroot}/wp-includes/version.php" ]; then
                            version=$(grep "\$wp_version =" "${docroot}/wp-includes/version.php" | cut -d\' -f2)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}WordPress ${version}${normal}\n"
                        fi
                        continue
                        ;;

                    Drupal)
                        # retrieve the version from the top of the changelog
                        if [ -f "${docroot}/CHANGELOG.txt" ]; then
                            version=$(head -n 1 "${docroot}/CHANGELOG.txt")
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}${version}${normal}\n"
                        fi
                        continue
                        ;;

                    Magento)
                        # retrieve the version from the top of the changelog
                        if [ -f "${docroot}/CHANGELOG.md" ]; then
                            version=$(head -n 1 "${docroot}/CHANGELOG.md")
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}Magento ${version}${normal}\n"
                        fi
                        continue
                        ;;
                      
                    PrestaShop)
                        if [ -f "${docroot}/src/Core/Version.php" ]; then
                            # PrestaShop 8.x
                            version=$(grep "public const VERSION =" "${docroot}/src/Core/Version.php" | cut -d\' -f2)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}PrestaShop ${version}${normal}\n"
                        elif [ -f "${docroot}/app/AppKernel.php" ]; then
                            # PrestaShop 1.7
                            version=$(grep "const VERSION =" "${docroot}/app/AppKernel.php" | cut -d\' -f2)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}PrestaShop ${version}${normal}\n"
                        elif [ -f "${docroot}/config/settings.inc.php" ]; then
                            # Prestashop 1.6 or earlier
                            version=$(grep "define('_PS_VERSION_'" "${docroot}/config/settings.inc.php" | cut -d\' -f4)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}PrestaShop ${version}${normal}\n"
                        fi
                        continue
                        ;;
                        
                    Joomla)
                        if [ -f "${docroot}/administrator/manifests/files/joomla.xml" ]; then
                            # joomla 2/3/4/5 we can get the version number from administrator/manifests/files/joomla.xml:
                            version=$(xmllint --xpath '//version/text()' "${docroot}/administrator/manifests/files/joomla.xml")
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}Joomla ${version}${normal}\n"
                        elif [ -f "${docroot}/includes/version.php" ]; then
                            # joomla 1.7 - includes/version.php 
                            # public $RELEASE = '1.7';
                            # public $DEV_LEVEL = '5';
                            release=$(grep "\$RELEASE" "${docroot}/includes/version.php" | cut -d\' -f 2)
                            dev=$(grep "\$DEV_LEVEL" "${docroot}/includes/version.php" | cut -d\' -f 2)
                            echo -en "${docroot} - ${green}Joomla ${release}.${dev}${normal}\n"
                        elif [ -f "${docroot}/libraries/joomla/version.php" ]; then
                            # joomla 1.5/1.6 - libraries/joomla/version.php
                            # public $RELEASE = '1.6';
                            # public $DEV_LEVEL       = '6';
                            release=$(grep "\$RELEASE" "${docroot}/libraries/joomla/version.php" | cut -d\' -f 2)
                            dev=$(grep "\$DEV_LEVEL" "${docroot}/libraries/joomla/version.php" | cut -d\' -f 2)
                            echo -en "${docroot} - ${green}Joomla ${release}.${dev}${normal}\n"
                        fi
                        continue
                        ;;

                    concrete)
                        # concrete/config/concrete.php
                        # 'version' => '8.5.19',
                        if [ -f "${docroot}/concrete/config/concrete.php" ]; then
                            version=$(grep "'version'" "${docroot}/concrete/config/concrete.php" | cut -d\' -f 4)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}Concrete CMS ${version}${normal}\n"
                        fi
                        continue
                        ;;

                    phpBB)
                        # includes/constants.php
                        # @define('PHPBB_VERSION', '3.3.14');
                        if [ -f "${docroot}/includes/constants.php" ]; then
                            version=$(grep "'PHPBB_VERSION'" "${docroot}/includes/constants.php" | cut -d\' -f 4)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}phpBB ${version}${normal}\n"
                        fi
                        continue
                        ;;

                    MediaWiki)
                        # includes/Defines.php 
                        # define( 'MW_VERSION', '1.42.3' );
                        if [ -f "${docroot}/includes/Defines.php" ]; then
                            version=$(grep "'MW_VERSION'" "${docroot}/includes/Defines.php" | cut -d\' -f 4)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}MediaWiki ${version}${normal}\n"
                        fi
                        continue
                        ;;

                    doku.php)
                        # DocuWiki
                        # version is stored in the VERSION file
                        if [ -f "${docroot}/VERSION" ]; then
                            version=$(<"$docroot/VERSION")
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}DocuWiki ${version}${normal}\n"
                        fi
                        continue
                        ;;

                    LARAVEL)
                        # Laravel
                        # vendor/laravel/framework/src/Illuminate/Foundation/Application.php
                        # const VERSION = '11.32.0';
                        if [ -f "${docroot}/vendor/laravel/framework/src/Illuminate/Foundation/Application.php" ]; then
                            version=$(grep "const VERSION" "${docroot}/vendor/laravel/framework/src/Illuminate/Foundation/Application.php" | cut -d\' -f 2)
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}Laravel ${version}${normal}\n"
                        fi
                        continue
                        ;;

                    PassengerNodejs)
                        echo -en "${docroot} - ${green}Node.js App${normal}\n"
                        continue
                        ;;

                    PassengerPython)
                        echo -en "${docroot} - ${green}Python App${normal}\n"
                        continue
                        ;;

                    WHMCS)
                        if [ -f "${docroot}/init.php" ]; then
                            version=$(grep "^\/\/ \* Version:" "${docroot}/init.php" | cut -d: -f2 | cut -d' ' -f2 | tr -d '()')
                            if [ -z "${version}" ]; then
                                version="?"
                            fi
                            echo -en "${docroot} - ${green}WHMCS ${version}${normal}\n"
                        fi
                        continue
                        ;;

                    *)
                        echo "Unknown app ${app}! This should never happen."
                        ;;
                esac
            fi
        done
    done
}

# process command line options
TEMP=$(getopt -o d:h --long depth,help -n 'cmsfinder.sh' -- "$@")

if [ $? != 0 ] ; then exit 1 ; fi

eval set -- "$TEMP"

while true; do
case "$1" in
    -d | --depth ) 
        searchdepth="${2}"
        shift 2
        ;;
    -- )
        shift
        break
        ;;
    * )
        usage
        exit 0
        ;;
esac
done

if [ -z "${1}" ]; then
    usage
    exit 0
else
    user="${1}"
    findapps
fi

