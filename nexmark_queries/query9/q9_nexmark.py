from windflow_table_api import *
from pathlib import Path

env = TableEnvironment(
    include_dir= Path("../../include"),
    par= 3, 
    policy=TimePolicy.EVENT_TIME,
    time_baseline=("2026-09-01T00:00:00.000Z", TimeFormats.ISO8601)
)

#---- auction
auction_schema = (SchemaBuilder()
    .add_column("auction_id", DataTypes.BIGINT)
    .add_column("item_name", DataTypes.STRING)
    .add_column("description", DataTypes.STRING)
    .add_column("initial_bid", DataTypes.BIGINT)
    .add_column("reserve", DataTypes.BIGINT)
    .add_column("auction_dateTime", TimeFormats.ISO8601)    #timestamp
    .add_column("expires", TimeFormats.ISO8601)
    .add_column("seller", DataTypes.BIGINT)                 #id di un Person
    .add_column("category", DataTypes.BIGINT)
    .build()
)

auction_config = InputFileConfiguration(
    path = Path("../../nexmark_datasets/30m_auction.csv"),
    format= FileFormat.CSV,
    schema= auction_schema,
    has_header= True,
    time_col= "auction_dateTime",
    order= True,                        
    split_size= SplitSize.megabytes(16)
)

auction = env.table_from_file(auction_config, "auction_source")

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
    path = Path("../../nexmark_datasets/30m_bid.csv"),
    format = FileFormat.CSV,
    schema = bid_schema,
    has_header = True,
    time_col = "bid_dateTime",
    order = True,                                           # da vedere
    split_size= SplitSize.megabytes(64)
)

bid = env.table_from_file(bid_config, "bid_source")

#---    QUERY 9
#--- Find the winning bid for each auction.

#le aste attuali durano meno di un giorno
interval = Interval(Duration.hours(-24), Duration.hours(1))

valid_bid = (
    (col("auction_dateTime") < col("bid_dateTime"))
    & (col("bid_dateTime") < col("expires"))
)

q9 = (bid
    .name_query("winning_bids")
    .join(auction, on = ["auction_id"] ,attachment=interval, where= valid_bid)
    .group_by("auction_id", window= Window.createTBWindow(Duration.days(2)))
    .select("auction_id", max("price").alias("winning_price"))
)

env.execute(q9, output_dir= "./winning")
