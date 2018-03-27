
typedef struct spell {
  float might;
  int mpcost;
} spell_t;


typedef struct chara {
  float hp;
  int mp;
  float mdef;
} chara_t;


float cast(spell_t *spell, chara_t *caster, chara_t *enemy)
{
  caster->mp -= spell->mpcost;
  __attribute((annotate("force_no_float"))) float might = spell->might;
  __attribute((annotate("force_no_float"))) float *ehp = &(enemy->hp);
  *ehp -= (might - enemy->mdef);
  return *ehp;
}


