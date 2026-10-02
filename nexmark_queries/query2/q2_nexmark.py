from windflow_tide import *
from pathlib import Path

env = TableEnvironment(
    include_dir= Path("../include"),
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
    path = Path("../nexmark_datasets/70m_bid.csv"),
    format = FileFormat.CSV,
    schema = bid_schema,
    has_header = True,
    time_col = "bid_dateTime",
    order = True,                                           # da vedere
    split_size= SplitSize.megabytes(128)
)

bid = env.table_from_file(bid_config, "bid_source")

#---    QUERY 2
#--- Find bids with specific auction ids and show their bid price.

cond = (
    (col("auction_id") == 1007)
    | (col("auction_id") == 1020)
    | (col("auction_id") == 2001)
    | (col("auction_id") == 2019)
    | (col("auction_id") == 2087)
)

q2 = (bid
    .name_query("specific_id")
    .where(cond)
    .select("auction_id", "price")
)

env.execute(q2, output_dir="./query2/filter_projection")
