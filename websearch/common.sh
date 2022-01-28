common_usage() {
  echo "$0 [config|start|stop] OPTIONS"
}

common_main() {
  case $1 in
    config)
      shift 1
      config "$@"
      ;;
    start)
      shift 1
      start "$@"
      ;;
    stop)
      shift 1
      stop "$@"
      ;;
    *)
      common_usage
      ;;
  esac
}