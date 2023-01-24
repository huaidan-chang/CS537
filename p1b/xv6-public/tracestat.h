struct tracestat
{
  int trace_enabled;    // enable tracing count of opening file
  char *trace_pathname; // pathname of tracing file
  int trace_count;      // count of opening tracing file
};

extern struct tracestat GLOBAL_STAT;




