from windflow_table_api import *
from pathlib import Path

env = TableEnvironment(
    include_dir= Path("../../include"),
    par= 2, 
    policy=TimePolicy.EVENT_TIME,
    time_baseline=("2026-09-01T00:00:00.000Z", TimeFormats.ISO8601)
)

#---- bid
bid_schema = (SchemaBuilder()
    .add_column("auction_id", DataTypes.BIGINT)             # Riferimento ad Auction
    .add_column("bidder", DataTypes.BIGINT)                 # ID utente (Person) che fa l'offerta
    .add_column("price", DataTypes.BIGINT)                  # Valore offerta (in centesimi, coerente con initial_bid/reserve)
    .add_column("channel", DataTypes.STRING)                # Canale di provenienza offerta
    .add_column("url", DataTypes.STRING)
    .add_column("bid_dateTime", TimeFormats.ISO8601)        # Timestamp dell'offerta
    .add_column("extra", DataTypes.STRING)                   # Campo di padding standard NEXMark
    .build()
)

bid_config = InputFileConfiguration(
    path = Path("../../nexmark_datasets/70m_bid.csv"),
    format = FileFormat.CSV,
    schema = bid_schema,
    has_header = True,
    time_col = "bid_dateTime",
    order = True,                                           # da vedere
    split_size= SplitSize.megabytes(128)
)

bid = env.table_from_file(bid_config, "bid_source")


#---    QUERY 1
#--- Convert each bid value from dollars to euros.

dol_to_eur = col("price") * 0.87

q1 = (bid
    .name_query("dol_to_eur")
    .select(
        "auction_id", dol_to_eur.alias("price_eur"), "bidder", "bid_dateTime"
    )
)

env.execute(q1, rexecute=True, output_dir="./to-eur")
