#ifndef PAWK_CHECK_H_
#define PAWK_CHECK_H_

// Use (void) to silent unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

#define S1(s) #s
#define S2(s) S1(s)
#define COORDS __FILE__ ":" S2(__LINE__) ": "
#define CK2(e, msg) do { if (!(e)) { fprintf(stderr, "%s failed: %s '%s'\n", COORDS, #e, msg); abort(); } } while (0)
#define CK3(e, msg1, msg2) CK2(e, msg1 + " (" + msg2 + ")")
#define CK(e) CK2(e, "")
#define FAIL(msg) do { fprintf(stderr, "%s failed: %s\n", COORDS, msg); abort(); } while (0)

#endif // PAWK_CHECK_H_
