from windflow_table_api import *
from pathlib import Path

env = TableEnvironment(
    include_dir= Path("../../include"),
    par= 5, 
    policy=TimePolicy.EVENT_TIME,
    time_baseline=("2026-09-01T00:00:00.000Z", TimeFormats.ISO8601)
)

#---- person
person_schema = (SchemaBuilder()
    .add_column("person_id", DataTypes.BIGINT)              # ID univoco dell'utente
    .add_column("name", DataTypes.STRING)
    .add_column("email_address", DataTypes.STRING)
    .add_column("credit_card", DataTypes.STRING)
    .add_column("city", DataTypes.STRING)
    .add_column("state", DataTypes.STRING)
    .add_column("person_dateTime", TimeFormats.ISO8601)     # Timestamp di registrazione
    .add_column("extra", DataTypes.STRING)                   # Campo di padding standard NEXMark
    .build()
)

person_config = InputFileConfiguration(
    path = Path("../../nexmark_datasets/70m_person.csv"),
    format = FileFormat.CSV,
    schema = person_schema,
    has_header = True,
    time_col = "person_dateTime",
    order = True,                                           # da vedere
    split_size= SplitSize.megabytes(8)
)

person = env.table_from_file(person_config, "person_source")

base = person.name_query("discard_p").select("person_id", "name", "email_address", "credit_card", "city", "state", "person_dateTime", "extra")

env.execute(base, output_dir= "./person")