//H(key || H(key || message)) = hmac
//H = ?
//key = ?
//length (d) = 4

//totpval(K) =  hotpval(k, ct)
//ct = (T-t0/tx) tx = duration

/*
 * Step 1:
 * HMAC(K,C) = H(K || H(K||C))
 * K is a private key shared between token/server
 * C is the counter
 *
 * Step 2:
 * HOTPVal = Trunc(HMAC(K,C)) (to 4 digits in our case)
 * ->
 * Let Snum  = StToNum(Sbits)
 * Return D = Snum mod 10^Digit
 *
 * Step 3:
 *
 * If Ct is  t-t0/tx and synced, then totp(Ct) = hotpval(K,ct)
 *
 *
 */
