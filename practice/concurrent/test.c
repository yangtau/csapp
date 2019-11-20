volatile long cnt;
void* fn() {
  cnt++;
  return fn;
}
