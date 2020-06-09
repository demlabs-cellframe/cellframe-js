for pidfile in node_*/node_*.pid; do kill `cat $pidfile`; done

